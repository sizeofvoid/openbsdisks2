/*
    Copyright 2016-2019 Gleb Popov <6yearold@gmail.com>

    Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>

#include "blockfilesystem.h"
#include "blockpartition.h"
#include "blockparttable.h"

class Block : public QObject, public QDBusContext
{
    Q_OBJECT

public:
    Block(const QString&);
    Block() = default;

    const QDBusObjectPath getDbusPath() const;

    QString getName() const;

    void setRegistered(bool);
    bool isUnregistered() const;

    void addPartition(const TBlockPartition&);
    TBlockPartition getPartition() const;

    void addPartitionTable(const TBlockPartTable&);
    TBlockPartTable getPartitionTable() const;

    QString driveName() const;

    Q_PROPERTY(QByteArray Device READ device)
    QByteArray device() const;

    Q_PROPERTY(qulonglong DeviceNumber READ deviceNumber)
    qulonglong deviceNumber() const;

    Q_PROPERTY(QByteArray PreferredDevice READ preferredDevice)
    QByteArray preferredDevice() const;

    Q_PROPERTY(QString IdLabel READ idLabel)
    QString idLabel() const;
    void setIdLabel(const QString&);

    Q_PROPERTY(QByteArrayList Symlinks READ symlinks)
    QByteArrayList symlinks();

    Q_PROPERTY(QString Id READ id)
    QString id() const;
    void setId(const QString&);

    Q_PROPERTY(QString IdType READ getIdType)
    QString getIdType() const;
    void setIdType(const QString&);

    Q_PROPERTY(QString IdUsage READ getIdUsage)
    QString getIdUsage() const;
    void setIdUsage(const QString&);

    Q_PROPERTY(qulonglong Size READ getSize)
    qulonglong getSize() const;
    void setSize(qulonglong);

    Q_PROPERTY(bool HintIgnore READ hintIgnore)
    bool hintIgnore() const;

    Q_PROPERTY(QString HintName READ hintName)
    QString hintName() const;

    // =======================================
    // Filesystem

    Q_PROPERTY(QByteArrayList MountPoints READ mountPoints);
    QByteArrayList mountPoints() const;

    // =======================================
    // PartitionTable

    Q_PROPERTY(QString PartitionTable_Type READ partitionTableType)
    QString partitionTableType();

    // =======================================
    // Partition

    Q_PROPERTY(QString Partition_Name READ partitionName)
    QString partitionName() const;

    Q_PROPERTY(uint Number READ number)
    uint number() const;

    Q_PROPERTY(qulonglong Offset READ offset)
    qulonglong offset() const;

    Q_PROPERTY(qulonglong Partition_Size READ partitionSize)
    qulonglong partitionSize() const;

    Q_PROPERTY(QString Partition_Type READ partitionType)
    QString partitionType() const;

    Q_PROPERTY(QDBusObjectPath Table READ table)
    QDBusObjectPath table() const;

    // =======================================
    // Unimplemented Block stuff

    Q_PROPERTY(QDBusObjectPath CryptoBackingDevice READ cryptoBackingDevice)
    QDBusObjectPath cryptoBackingDevice() const
    {
        return QDBusObjectPath("/");
    }

    Q_PROPERTY(QDBusObjectPath Drive READ drive)
    QDBusObjectPath drive() const;

    Q_PROPERTY(QDBusObjectPath MDRaid READ mDRaid)
    QDBusObjectPath mDRaid() const
    {
        return QDBusObjectPath("/");
    }

    Q_PROPERTY(QDBusObjectPath MDRaidMember READ mDRaidMember)
    QDBusObjectPath mDRaidMember() const
    {
        return QDBusObjectPath("/");
    }

public slots:
    QString Mount(const QVariantMap& options)
    {
        if (!getPartition() && !getPartition()->getFilesystem())
            return QString();

        return getPartition()->getFilesystem()->Mount(*this, options, connection(), message());
    }

    void Unmount(const QVariantMap& options)
    {
        if (!getPartition() && !getPartition()->getFilesystem())
            return;

        return getPartition()->getFilesystem()->Unmount(*this, options, connection(), message());
    }

private:
    QString m_IdType;

    QString m_IdUsage;

    qulonglong m_Size;

    const QString m_Name;

    const QDBusObjectPath m_dbusPath;

    bool m_Registered = false;

    QString m_Description;

    QString m_Id;

    QString m_IdLabel;

    QStringList m_Lavels;

    bool m_HasNoDrive = false;

    // org.freedesktop.UDisks2.Partition — Block device representing a partition
    TBlockPartition m_Partition;
    // org.freedesktop.UDisks2.PartitionTable — Block device containing a
    // partition table
    TBlockPartTable m_PartTable;

    // org.freedesktop.UDisks2.Swapspace — Block device containing swap data
    // org.freedesktop.UDisks2.Encrypted — Block device containing encrypted
    // data org.freedesktop.UDisks2.Loop — Block device backed by a file
    /**
     * TODO Not impl
     *
     * Device                 readable   ay
     * ReadOnly               readable   b
     * IdVersion              readable   s
     * IdUUID                 readable   s
     * Configuration          readable   a(sa{sv})
     * HintPartitionable      readable   b
     * HintSystem             readable   b
     * HintIgnore             readable   b
     * HintAuto               readable   b
     * HintIconName           readable   s
     * HintSymbolicIconName   readable   s
     * UserspaceMountOptions  readable   as
     */
};

using TBlock = std::shared_ptr<Block>;
using TBlockVec = std::vector<TBlock>;
