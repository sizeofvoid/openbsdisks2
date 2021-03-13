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

#include "disk_label.h"
#include "block.h"
#include "blockpartition.h"
#include "bsdisks.h"

#define DKTYPENAMES
#include <sys/disklabel.h>

#include <fcntl.h>
#include <sys/dkio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <util.h>

#include <QChar>
#include <QDebug>
#include <QString>

DiskLabel::DiskLabel(const QString& dev)
{
    analyseDev(dev);
}

void DiskLabel::analyseDev(const QString& dev)
{
    struct disklabel lab;
    char* specname;
    int f = opendev(dev.toLocal8Bit().data(),
        O_RDONLY,
        OPENDEV_PART,
        &specname);

    if (ioctl(f, DIOCGDINFO, &lab) == -1) {
        close(f);
        return;
    }

    createDrive(dev);

    struct disklabel::partition* pp = nullptr;

    for (int i = 0; i < lab.d_npartitions; i++) {
        pp = &lab.d_partitions[i];
        if (DL_GETPSIZE(pp)) {
            QChar p('a' + i);
            if (p != QChar('c')) {
                if (isValidFileSysetem(pp->p_fstype)) {
                    createBlock(QString(p), QString(fstypesnames[pp->p_fstype]));
                }
            }
        }
    }
    close(f);
}

bool DiskLabel::isValidFileSysetem(u_int8_t fstype) const
{
    switch (fstype) {
    case FS_UNUSED:
    case FS_SWAP:
    case FS_V6:
    case FS_V7:
    case FS_SYSV:
    case FS_V71K:
    case FS_V8:
        return false;

    case FS_BSDFFS:
    case FS_MSDOS:
        return true;

    case FS_BSDLFS:
    case FS_OTHER:
    case FS_HPFS:
        return false;
    case FS_ISO9660:
        return true;

    case FS_BOOT:
    case FS_ADOS:
    case FS_HFS:
    case FS_ADFS:
        return false;

    case FS_EXT2FS:
        return true;

    case FS_CCD:
    case FS_RAID:
        return false;

    case FS_NTFS:
        return true;

    case FS_UDF:
        return false;
    };
    return false;
}

TDrive
DiskLabel::getDrive() const
{
    return m_drive;
}

QString
DiskLabel::getDeviceName() const
{
    assert(getDrive());
    return getDrive() ? getDrive()->getDeviceName() : QString();
}

bool DiskLabel::isValid() const
{
    return m_drive != nullptr;
}

void DiskLabel::createDrive(const QString& dev)
{
    qDebug() << "Created drive " << dev;
    m_drive = std::make_shared<Drive>(dev);
}

void DiskLabel::createBlock(const QString& partitionNumber, const QString& fstype)
{
    const QString dev(getDeviceName() + partitionNumber);
    auto block = std::make_shared<Block>(dev);

    auto partition = std::make_shared<BlockPartition>();
    block->addPartition(partition);

    auto bfs = std::make_shared<BlockFilesystem>();
    bfs->filesystem = fstype;
    block->idUsage = QStringLiteral("filesystem");
    block->idType = fstype;
    ;

    for (const QStorageInfo& storage : QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            if (!storage.device().compare(block->device().chopped(1))) {
                bfs->mountPoints << storage.rootPath().toLocal8Bit();
                break;
            }
        }
    }
    block->getPartition()->addFilesystem(bfs);
    m_drive->addBlock(block);
}
