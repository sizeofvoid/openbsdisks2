/*
 * Copyright 2020 Rafael Sadowski <rafael@sizeofvoid.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "disk_thread.h"
#include "disk_label.h"

#include <QChar>
#include <QString>

#include <QDebug>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/disklabel.h>
#include <sys/dkio.h>
#include <sys/ioctl.h>
#include <util.h>

#include <memory>

QString DiskThread::readDisknames() const
{
    size_t len = 0;
    int    mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_DISKNAMES;

    if (sysctl(mib, 2, nullptr, &len, nullptr, 0) == -1) {
        qCritical("can't get hw.disknames");
        return QString();
    }

    if (len > 0) {
        char* disknames = (char*)malloc(len);

        if (sysctl(mib, 2, disknames, &len, nullptr, 0) == -1) {
            qCritical("can't get hw.disknames");
            return QString();
        }

        const QString disks(disknames);
        delete disknames;
        return disks;
    }
    return QString();
}

void DiskThread::run()
{
    m_t = new QTimer();
    connect(m_t, SIGNAL(timeout()), this, SLOT(check()));
    m_t->start(1000);
    exec();
}

void DiskThread::check()
{
    // "sd0:6e6c992178f67d41,sd2:0f191ebc5bc2aa61,sd1:"
    const QString disks = readDisknames();
    const auto    devNameUuids = getCurrentDev(disks);

    for (const auto& devNameUuid : devNameUuids)
        addNewDevices(devNameUuid.first);

    std::vector<QString> toDelete;
    for (auto const& dl : diskLabels) {
        if (std::none_of(std::begin(devNameUuids), std::end(devNameUuids), [&](auto const& du) -> bool {
                return du.first == dl->getDeviceName();
            })) {
            toDelete.push_back(dl->getDeviceName());
            emit deviceRemoved(dl->getDrive());
        }
    }
    for (auto const& del : toDelete) {
        removeDevices(del);
    }
}

void DiskThread::removeDevices(const QString& devName)
{
    diskLabels.erase(std::remove_if(diskLabels.begin(),
                                    diskLabels.end(),
                                    [&](TDiskLabel const& d) -> bool { return devName == d->getDeviceName(); }),
                     diskLabels.end());
}

void DiskThread::addNewDevices(const QString& devName)
{
    if (std::find_if(std::begin(diskLabels), std::end(diskLabels), [&](TDiskLabel const& d) -> bool {
            return devName == d->getDeviceName();
        }) == std::end(diskLabels)) {
        auto dl = std::make_shared<DiskLabel>(devName);
        if (dl->isValid()) {
            diskLabels.push_back(dl);
            emit deviceAdded(dl->getDrive());
            for (const TBlock& block : dl->getDrive()->getBlocks())
                emit blockAdded(block);
        }
    }
}

std::vector<std::pair<QString, QString>> DiskThread::getCurrentDev(const QString& disks)
{
    std::vector<std::pair<QString, QString>> devNameUuids;
    // "sd0:6e6c992178f67d41,sd2:0f191ebc5bc2aa61,sd1:"

    const QStringList nameUuids = disks.split(QLatin1Char(','));
    for (QString const& nameUuid : nameUuids) {
        const QStringList name2Uuid = nameUuid.split(QLatin1Char(':'));
        // We want both name2Uuid und uuid to verify valid devices
        if (name2Uuid.size() >= 1) {
            QString dev = name2Uuid.at(0);
            devNameUuids.emplace_back(name2Uuid.at(0), name2Uuid.at(1));
        }
    }
    return devNameUuids;
}
