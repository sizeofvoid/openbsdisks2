/*
    Copyright 2016-2019 Gleb Popov <6yearold@gmail.com>
    Copyright 2020-2021 Rafael Sadowski <rs@rsadowski.de>

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

#pragma once

#include <QDBusObjectPath>
#include <QObject>

#include "adaptors.h"
#include "block.h"

class QUuid;

/**
 *
 * org.freedesktop.UDisks2.Drive
 *
 * This interface is used to represent both hard disks and disk drives (with
 * or without removable media).
 *
 * This interface should not to be confused with the
 * org.freedesktop.UDisks2.Block interface that is used for low-level block
 * devices the OS knows about. For example, if /dev/sda and /dev/sdb are block
 * devices for two paths to the same drive, there will be only one
 * org.freedesktop.UDisks2.Drive object but two org.freedesktop.UDisks2.Block
 * objects.
 *
 * http://storaged.org/doc/udisks2-api/latest/gdbus-org.freedesktop.UDisks2.Drive.html
 */
class Drive : public QObject,
              public QDBusContext {
    Q_OBJECT
public:
    Drive() = default;
    Drive(const QString&);

    const QDBusObjectPath getDbusPath() const;

    QString getDeviceName() const;

    void setDescription(const QString&);

    void setRemovable(bool);

    void addBlock(const TBlock&);
    const TBlockVec getBlocks() const;

    void setDuid(const QUuid&);

    Q_PROPERTY(Configuration Configuration READ configuration)
    Configuration configuration() const;

    Q_PROPERTY(QString Vendor READ vendor)
    QString vendor() const;

    Q_PROPERTY(qulonglong Size READ driveSize)
    qulonglong driveSize() const;
    void setSize(qulonglong);

    Q_PROPERTY(QString Serial READ serial)
    QString serial() const;

    Q_PROPERTY(QStringList MediaCompatibility READ mediaCompatibility)
    QStringList mediaCompatibility() const;

    Q_PROPERTY(bool Optical READ optical)
    bool optical() const;

    Q_PROPERTY(bool Ejectable READ ejectable)
    bool ejectable() const;

    Q_PROPERTY(bool Removable READ removable)
    bool removable() const;

    Q_PROPERTY(bool MediaRemovable READ mediaRemovable)
    bool mediaRemovable() const;

    Q_PROPERTY(QString ConnectionBus READ connectionBus)
    QString connectionBus() const;

public slots:
    void Eject(const QVariantMap& options);

private:
    TBlockVec m_blocks;

    const QString m_deviceName;

    const QDBusObjectPath m_dbusPath;

    QString m_Description;

    bool isRemovable = false;
    qulonglong size;

    // disklabel(8) UID
    QUuid m_Duid;
};

using TDrive = std::shared_ptr<Drive>;
using TDriveVec = std::vector<TDrive>;
