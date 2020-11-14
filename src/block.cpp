/*
    Copyright 2016 Gleb Popov <6yearold@gmail.com>

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors may
    be used to endorse or promote products derived from this software without specific
    prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
    OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/stat.h>

#include <QFileInfo>

#include "block.h"
#include "bsdisks.h"

Block::Block(QObject* parent)
    : QObject(parent)
    , bFilesystem(nullptr)
    , bPartition(nullptr)
    , bPartTable(nullptr)
    , needsAnotherProbe(false)
    , registered(false)
    , hasNoDrive(false)
{
}

QString Block::id() const
{
    if (bPartition) {
        auto tableType = bPartition->partTableBlock->bPartTable->tableType();
        QChar partitionSymbol = tableType == QStringLiteral("gpt")
            ? 'p'
            : 's';
        return bPartition->partTableBlock->id() + "_" + name.mid(name.lastIndexOf(partitionSymbol));
    }
    else
        return description + "_" + identifier;
}

QString Block::idLabel() const
{
    if (bFilesystem && bFilesystem->filesystem == "zfs")
        return bFilesystem->zfsDataset;

    return labels.empty()
        ? QString()
        : QUrl::fromPercentEncoding(labels[0].mid(labels[0].lastIndexOf('/') + 1).toLatin1());
}

QString Block::driveName() const
{
    if (bPartition)
        return bPartition->partTableBlock->driveName();

    return name;
}

QDBusObjectPath Block::drive() const
{
    return QDBusObjectPath(UDisksDrives + driveName());
}

QByteArray Block::preferredDevice() const
{
    if (labels.empty())
        return device();
    return (QStringLiteral("/dev/") + labels[0]).toLocal8Bit() + '\0';
}

QByteArray Block::device() const
{
    return (QStringLiteral("/dev/") + name).toLocal8Bit() + '\0';
}

qulonglong Block::deviceNumber() const
{
    struct stat st;

    ::stat((QStringLiteral("/dev/") + name).toLocal8Bit().constData(), &st);
    return st.st_rdev;
}

QByteArrayList Block::symlinks()
{
    QByteArrayList r;
    foreach (auto s, labels)
        r << (QStringLiteral("/dev/") + s).toLocal8Bit() + '\0';
    return r;
}

qulonglong Block::blockSize() const
{
    return bPartition ? bPartition->size : size;
}

bool Block::hintIgnore() const
{
    if (bPartition) {
        // there should be better predicate for this, probably
        // right now ignore efi partitions on ada/ad, these usually contain bootloader
        // stuff and not needed to be mounted by regular user
        QFileInfo devInfo(device());
        if (bPartition->partitionType == "efi" && devInfo.fileName().startsWith("ad"))
            return true;
    }

    return false;
}

QString Block::hintName() const
{
    return idLabel();
}

QByteArrayList Block::mountPoints() const
{
    if (bFilesystem) {
        QByteArrayList r;

        for (auto mp : bFilesystem->mountPoints)
            r << mp + '\0';

        return r;
    }

    return QByteArrayList();
}

QString Block::partitionTableType()
{
    if (!bPartTable)
        return QString();

    return bPartTable->tableType();
}

QString Block::partitionName() const
{
    if (!bPartition)
        return QString();

    return bPartition->name();
}

uint Block::number() const
{
    if (!bPartition)
        return 0;

    return bPartition->number;
}

qulonglong Block::offset() const
{
    if (!bPartition)
        return 0;

    return bPartition->offset;
}

qulonglong Block::partitionSize() const
{
    if (!bPartition)
        return 0;

    return bPartition->size;
}

QString Block::partitionType() const
{
    if (!bPartition)
        return 0;

    return bPartition->partitionType;
}

QDBusObjectPath Block::table() const
{
    if (!bPartition)
        return QDBusObjectPath("/");

    return bPartition->partTableBlock->dbusPath;
}
