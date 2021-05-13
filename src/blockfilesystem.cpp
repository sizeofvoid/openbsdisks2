/*
    Copyright 2016 Gleb Popov <6yearold@gmail.com>

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

#include "blockfilesystem.h"
#include "block.h"
#include "bsdisks.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QProcess>

#include <sys/mount.h>
#include <sys/param.h>
#include <sys/ucred.h>

#include <errno.h>
#include <unistd.h>

static bool alreadyMounted(QDir d)
{
    struct statfs* buf;
    const int count = ::getmntinfo(&buf, MNT_NOWAIT);

    if (count <= 0) {
        QString error = QString::fromLocal8Bit(::strerror(errno));
        qDebug() << "alreadyMounted: getmntinfo failed: " << error;
        return false;
    }

    for (int i = 0; i < count; i++) {
        if (d == QDir(QString::fromLocal8Bit(buf[i].f_mntonname)))
            return true;
    }

    return false;
}

static QString createMountPoint(const QString& id, uid_t uid, int exists = 0)
{
    const QString mediaDir = exists == 0 ? QStringLiteral("/media/") + id
                                         : QStringLiteral("/media/") + id + QString::number(exists);

    if (QDir(mediaDir).exists() && !alreadyMounted(mediaDir))
        return mediaDir;

    if (QDir(mediaDir).exists() && alreadyMounted(mediaDir))
        return createMountPoint(id, uid, ++exists);

    if (!QDir().mkdir(mediaDir)) {
        qDebug() << QString("Can't create the directory: ") + mediaDir;
        return QString();
    }

    if (::chown(mediaDir.toLocal8Bit().constData(), uid, -1)) {
        qDebug() << "createMountPoint: " << QString::fromLocal8Bit(::strerror(errno));
    }

    return mediaDir;
}

static void removeMountPoint(QString mp, bool checkIfEmpty = false)
{
    QDir mpDir(mp);
    if (checkIfEmpty && !mpDir.isEmpty())
        return;

    const auto dirName = mpDir.dirName();
    mpDir.cdUp();
    mpDir.rmdir(dirName);
}

QString BlockFilesystem::Mount(const Block& block,
                               const QVariantMap& options,
                               QDBusConnection conn,
                               const QDBusMessage& msg)
{
    if (!isFilesystemSupportedToMount()) {
        const QString error = "Filesystem is not supported to mount";
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", error));
        return QString();
    }

    // fail if already mounted
    if (!getMountPoints().empty()) {
        const QString error = "Mount: device already mounted";
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.AlreadyMounted", error));
        return QString();
    }

    // https://dbus.freedesktop.org/doc/dbus-specification.html#bus-messages-get-connection-credentials
    auto uidReply = conn.interface()->serviceUid(msg.service());
    if (!uidReply.isValid()) {
        const QString error = "Mount: interface.serviceUid() failed";
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", error));
        return QString();
    }

    QProcess mount;
    const QString mountPoint = createMountPoint(block.id().replace(' ', '_'), uidReply.value());
    if (mountPoint.isEmpty()) {
        const QString error = "Mount: failed with " + mount.readAllStandardError();
        removeMountPoint(mountPoint);
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", error));
        return QString();
    }

    const auto mountProg = getMountCommand();
    if (mountProg.isEmpty())
        return QString();

    mount.setProgram(mountProg);

    QStringList args;
    args += getMountOptions();
    args << QString("/dev/") + block.getName() << mountPoint;
    mount.setArguments(args);

    mount.start();
    mount.waitForFinished(-1);

    if (mount.exitCode()) {
        QString error = "Mount: failed with " + mount.readAllStandardError();
        qDebug() << error;
        removeMountPoint(mountPoint);
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", error));
        return QString();
    }

    addMountPoint(mountPoint);
    signalMountPointsChanged();
    return mountPoint;
}

void BlockFilesystem::Unmount(const Block& block,
                              const QVariantMap& options,
                              QDBusConnection conn,
                              const QDBusMessage& msg)
{
    if (getMountPoints().empty()) {
        const QString err = "Unmount: requested filesystem is not mounted";
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.NotMounted", err));
        qDebug() << err;
        return;
    }

    QStringList mountOptions;
    for (const QVariant& option : options) {
        if (!option.isNull())
            mountOptions << option.toString();
    }

    QStringListIterator mountIt(getMountPoints());
    while (mountIt.hasNext()) {
        const QString mountPoint = mountIt.next();
        QStringList args;
        if (!mountOptions.isEmpty())
            args << mountOptions;
        args << mountPoint;
        QProcess umount;
        umount.start(QStringLiteral("/sbin/umount"), args);
        umount.waitForFinished(-1);

        if (umount.exitCode()) {
            removeMountPoint(mountPoint, true);
            QString errorMessage = umount.readAllStandardError();
            conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", errorMessage));
            signalMountPointsChanged();
        } else {
            removeMountPoint(mountPoint, true);
            mountPoints.removeAll(mountPoint);
        }
    }
}

void BlockFilesystem::signalMountPointsChanged()
{
    QVariantMap props;
    props.insert(QStringLiteral("MountPoints"), QVariant::fromValue(getMountPoints().join(",")));

    QDBusMessage signal =
        QDBusMessage::createSignal("", // XX parentBlock()->dbusPath.path(),
                                   QStringLiteral("org.freedesktop.DBus.Properties"),
                                   QStringLiteral("PropertiesChanged"));

    signal << QStringLiteral("org.freedesktop.UDisks2.Filesystem") << props << QStringList();
    QDBusConnection::systemBus().send(signal);
}

const QStringList& BlockFilesystem::getMountPoints() const
{
    return mountPoints;
}

void BlockFilesystem::addMountPoint(const QString& mp)
{
    mountPoints << mp;
}

void BlockFilesystem::setFilesystem(const QString& fs)
{
    filesystem = fs;
}

bool BlockFilesystem::isFilesystemSupportedToMount() const
{
    return (filesystem == "ffs"
            || filesystem == "ext2fs"
            || filesystem == "ntfs"
            || filesystem == "msdos");
}

const QString BlockFilesystem::getMountCommand() const
{
    QString mountProg;
    if (filesystem == "ffs") {
        mountProg = QStringLiteral("/sbin/mount_ffs");
    }
    else if (filesystem == "ext2fs") {
        mountProg = QStringLiteral("/sbin/mount_ext2fs");
    }
    else if (filesystem == "ntfs") {
        mountProg = QStringLiteral("/sbin/mount_ntfs");
    }
    else if (filesystem == "ntfs") {
        mountProg = QStringLiteral("/sbin/mount_msdos");
    }
    return mountProg;
}

const QStringList BlockFilesystem::getMountOptions() const
{
    QStringList mountOps;
    if (filesystem == "ffs") {
        mountOps << QStringLiteral("-orw,nodev,nosuid,noatime");
    }
    else if (filesystem == "ext2fs") {
        mountOps << QStringLiteral("-orw,nodev,nosuid,noatime");
    }
    else if (filesystem == "msdos") {
        mountOps << QStringLiteral("-orw");
    }
    else if (filesystem == "ntfs") {
        mountOps << QStringLiteral("-oro");
    }
    return mountOps;
}
