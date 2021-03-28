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

#include <fcntl.h>
#include <sys/cdio.h>
#include <sys/ioctl.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusObjectPath>
#include <QUuid>

#include "bsdisks.h"
#include "drive.h"

Drive::Drive(const QString& devName)
    : QDBusContext()
    , m_deviceName(devName)
    , m_dbusPath(QDBusObjectPath(UDisksDrives + devName))
{
}

const QDBusObjectPath
Drive::getDbusPath() const
{
    return m_dbusPath;
}

QString
Drive::getDeviceName() const
{
    return m_deviceName;
}

void Drive::setDescription(const QString& d)
{
    m_Description = d;
}

void Drive::setRemovable(bool r)
{
    isRemovable = r;
}


void Drive::addBlock(const TBlock& block)
{
    qDebug() << "Disk " << getDeviceName() << " add block: " << block->getName();
    m_blocks.push_back(block);
}

const TBlockVec
Drive::getBlocks() const
{
    return m_blocks;
}

void Drive::Eject(const QVariantMap& options)
{
    if (!optical())
        return;

    int fd = open((QStringLiteral("/dev/") + getDeviceName()).toLocal8Bit().constData(), O_RDONLY);
    if (fd < 0 && errno != ENXIO) {
        QString errorMessage = ::strerror(errno);
        connection().send(message().createErrorReply("org.freedesktop.UDisks2.Error.Failed", errorMessage));
        qDebug() << "Eject failed: " << errorMessage;
        return;
    }

    ::ioctl(fd, CDIOCALLOW);
    int rc = ::ioctl(fd, CDIOCEJECT);
    if (rc < 0) {
        QString errorMessage = ::strerror(errno);
        connection().send(message().createErrorReply("org.freedesktop.UDisks2.Error.Failed", errorMessage));
        qDebug() << "Eject failed: " << errorMessage;
        return;
    }
}

Configuration Drive::configuration() const
{
    Configuration c;
    return c;
}

bool Drive::optical() const
{
    return getDeviceName().startsWith("cd");
}

QStringList Drive::mediaCompatibility() const
{
    if (optical())
        return {QStringLiteral("optical_cd")};
    return QStringList();
}

QString Drive::vendor() const
{
    return m_Description;
}

qulonglong Drive::driveSize() const
{
    return size;
}

QString Drive::serial() const
{
    return m_Duid.toString();
}

void
Drive::setSize(qulonglong s)
{
    size = s;
}

void
Drive::setDuid(const QUuid& duid)
{
    m_Duid = duid;
}

bool Drive::ejectable() const
{
    return isRemovable;
}

bool Drive::removable() const
{
    return isRemovable;
}

bool Drive::mediaRemovable() const
{
    return optical() || removable();
}

QString Drive::connectionBus() const
{
    return QString();
}
