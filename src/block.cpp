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

#include <QByteArrayList>
#include <QFileInfo>
#include <QDebug>

#include "block.h"
#include "bsdisks.h"

Block::Block(const QString& blockName)
    : QDBusContext()
    , m_Name(blockName)
    , m_dbusPath(QDBusObjectPath(UDisksBlockDevices + blockName))
{
}

const QDBusObjectPath
Block::getDbusPath() const
{
    return m_dbusPath;
}

QString
Block::getName() const
{
    return m_Name;
}
QString
Block::getIdType() const
{
    return m_IdType;
}

QString
Block::getIdUsage() const
{
    return m_IdUsage;
}
void Block::setIdType(const QString& id)
{
    m_IdType = id;
}

void Block::setIdUsage(const QString& usage)
{
    m_IdType = usage;
}

void
Block::setRegistered(bool reg)
{
    m_Registered = reg;
}

bool
Block::isUnregistered() const
{
    return !m_Registered;
}

void Block::addPartition(const TBlockPartition& partition)
{
    m_Partition = partition;
}

void Block::addPartitionTable(const TBlockPartTable& btt)
{
    m_PartTable = btt;
}

TBlockPartition
Block::getPartition() const
{
    return m_Partition;
}

TBlockPartTable
Block::getPartitionTable() const
{
    return m_PartTable;
}

QString Block::id() const
{
    if (getPartitionTable()) {
        auto tableType = getPartitionTable()->tableType();
        QChar partitionSymbol = tableType == QStringLiteral("gpt")
            ? 'p'
            : 's';
        return partitionSymbol;
        // XXX return getPartitionTable()->id() + "_" + getName().mid(getName().lastIndexOf(partitionSymbol));
    }
    else
        return m_Description + "_" + m_Id;
}

QString Block::idLabel() const
{
    return getName();
}

QString Block::driveName() const
{
    /*
    if (getPartition())
        return getPartitionTable()->driveName();
        */

    return getName();
}

QDBusObjectPath Block::drive() const
{
    return QDBusObjectPath(UDisksDrives + driveName());
}

QByteArray Block::preferredDevice() const
{
    if (m_Lavels.empty())
        return device();
    return (QStringLiteral("/dev/") + m_Lavels[0]).toLocal8Bit() + '\0';
}

QByteArray Block::device() const
{
    return (QStringLiteral("/dev/") + getName()).toLocal8Bit() + '\0';
}

qulonglong Block::deviceNumber() const
{
    struct stat st;

    ::stat((QStringLiteral("/dev/") + getName()).toLocal8Bit().constData(), &st);
    return st.st_rdev;
}

QByteArrayList Block::symlinks()
{
    QByteArrayList r;
    foreach (auto s, m_Lavels)
        r << (QStringLiteral("/dev/") + s).toLocal8Bit() + '\0';
    return r;
}

qulonglong Block::getSize() const
{
    return m_Size;
}

void Block::setSize(qulonglong size)
{
    m_Size = size;
}

bool Block::hintIgnore() const
{
    if (getPartition()) {
        // there should be better predicate for this, probably
        // right now ignore efi partitions on ada/ad, these usually contain bootloader
        // stuff and not needed to be mounted by regular user
        QFileInfo devInfo(device());
        if (getPartition()->partitionType == "efi" && devInfo.fileName().startsWith("ad"))
            return true;
    }

    return false;
}

QString Block::hintName() const
{
    return getName();
}

QByteArrayList Block::mountPoints() const
{
    if (getPartition() && getPartition()->getFilesystem()) {
        QByteArrayList r;

        for (auto mp : getPartition()->getFilesystem()->mountPoints)
            r << mp + '\0';

        return r;
    }

    return QByteArrayList();
}

QString Block::partitionTableType()
{
    if (!getPartitionTable())
        return QString();

    return getPartitionTable()->tableType();
}

QString Block::partitionName() const
{
    if (!getPartition())
        return QString();

    return getPartition()->name();
}

uint Block::number() const
{
    if (!getPartition())
        return 0;

    return getPartition()->number;
}

qulonglong Block::offset() const
{
    if (!getPartition())
        return 0;

    return getPartition()->offset;
}

qulonglong Block::partitionSize() const
{
    if (!getPartition())
        return 0;

    return getPartition()->size;
}

QString Block::partitionType() const
{
    if (!getPartition())
        return 0;

    return getPartition()->partitionType;
}

QDBusObjectPath Block::table() const
{
    if (!getPartition())
        return QDBusObjectPath("/");

    return m_dbusPath;
}
