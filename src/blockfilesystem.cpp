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

#include "bsdisks.h"
#include "blockfilesystem.h"
#include "block.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QProcess>
#include <QDir>

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>

#include <unistd.h>
#include <errno.h>

static bool alreadyMounted(QDir d)
{
    struct statfs* buf;
    int count = ::getmntinfo(&buf, MNT_NOWAIT);

    if(!count)
    {
        QString error = QString::fromLocal8Bit(::strerror(errno));
        qDebug() << "alreadyMounted: getmntinfo failed: " << error;
        return false;
    }

    for(int i = 0; i<count; i++)
    {
        QString mntOn = QString::fromLocal8Bit(buf[i].f_mntonname);
        if(d == QDir(mntOn))
            return true;
    }

    return false;
}

static QString createMountPoint(QString id, uid_t uid, int suff = 0)
{
    auto mp = suff == 0 ? QStringLiteral("/media/") + id
                        : QStringLiteral("/media/") + id + QString::number(suff);
    QDir mpDir(mp);

    if(mpDir.exists() && alreadyMounted(mpDir))
        return createMountPoint(id, uid, ++suff);

    mpDir.mkpath(".");

    qDebug() << "chown of " << mpDir.absolutePath() << " to " << QString::number(uid);
    if(::chown(mpDir.absolutePath().toLocal8Bit().constData(), uid, -1))
    {
        QString error = QString::fromLocal8Bit(::strerror(errno));
        qDebug() << "createMountPoint: " << error;
    }

    return mp;
}

static void removeMountPoint(QString mp, bool checkIfEmpty = false)
{
    QDir mpDir(mp);

    if(checkIfEmpty && mpDir.entryList().count() > 2) // '.' and '..' also counts
        return;

    auto dirName = mpDir.dirName();
    mpDir.cdUp();
    mpDir.rmdir(dirName);
}

QString BlockFilesystem::Mount(const QVariantMap& options)
{

    auto msg = parentBlock()->message();
    auto conn = parentBlock()->connection();

    // fail if already mounted
    if(mountPoints.count() > 0)
    {
        QString error = "Mount: device " + parentBlock()->id() + "already mounted";
        qDebug() << error;
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.AlreadyMounted", error));
        return QString();
    }

    // https://dbus.freedesktop.org/doc/dbus-specification.html#bus-messages-get-connection-credentials
    auto uidReply = conn.interface()->serviceUid(msg.service());
    if(!uidReply.isValid())
    {
        QString error = "Mount: interface.serviceUid() failed";
        qDebug() << error;

        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", error));
        return QString();
    }

    uid_t uid = uidReply.value();

    QProcess mount;
    QString mountPoint = filesystem == "zfs"
        ? zfsDataset
        : createMountPoint(parentBlock()->id().replace(' ', '_'), uid);
    QStringList args;

    auto mountProg = QStringLiteral("/sbin/mount");
    if(filesystem == "msdosfs")
    {
        mountProg = QStringLiteral("/sbin/mount_msdosfs");

        if(!BsdisksConfig::get().MountMsdosfsFlags.isEmpty())
            args << BsdisksConfig::get().MountMsdosfsFlags.split(' ');
    }
    else if(filesystem == "ntfs")
    {
        mountProg = QStringLiteral("ntfs-3g");
    }
    else if(filesystem == "cd9660")
    {
        mountProg = QStringLiteral("/sbin/mount_cd9660");
    }
    else if(filesystem == "ext2fs")
    {
        mountProg = QStringLiteral("fuse2fs");

        args << QStringLiteral("-o") << (QStringLiteral("uid=") + QString::number(uid));
        args << QStringLiteral("-o") << QStringLiteral("allow_other");
    }
    else if(filesystem == "exfat")
    {
        mountProg = QStringLiteral("mount.exfat-fuse");

        args << QStringLiteral("-o") << (QStringLiteral("uid=") + QString::number(uid));
        args << QStringLiteral("-o") << QStringLiteral("allow_other");
    }

    if(filesystem == "zfs")
    {
        mountProg = QStringLiteral("/sbin/zfs");

        args << QStringLiteral("mount") << mountPoint;
    }
    else
        args << parentBlock()->device() << mountPoint;

    mount.setProgram(mountProg);
    mount.setArguments(args);

    mount.start();
    mount.waitForFinished(-1);

    if(mount.exitCode())
    {
        QString error = "Mount: failed with " + mount.readAllStandardError();
        qDebug() << error;
        removeMountPoint(mountPoint);
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", error));
        return QString();
    }

    // For ZFS, the "mountPoint" variable contains dataset name
    if(filesystem == "zfs")
        mountPoints << zfsMountpoint.toLocal8Bit();
    else
        mountPoints << mountPoint.toLocal8Bit();

    signalMountPointsChanged();

    return mountPoint;
}

void BlockFilesystem::Unmount(const QVariantMap& options)
{
    auto msg = parentBlock()->message();
    auto conn = parentBlock()->connection();

    if(mountPoints.empty())
    {
        QString error = "Unmount: requested filesystem is not mounted";
        conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.NotMounted", error));
        qDebug() << error;
        return;
    }

    bool error = false;

    if(filesystem == "zfs")
    {
        QProcess zfsProcess;
        QStringList args;

        args << QStringLiteral("unmount") << zfsDataset;

        zfsProcess.start(QStringLiteral("/sbin/zfs"), args);
        zfsProcess.waitForFinished(-1);

        if(zfsProcess.exitCode() == 0)
        {
            mountPoints.clear();
            signalMountPointsChanged();
        }
        else
        {
            QString errorMessage = zfsProcess.readAllStandardError();
            conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", errorMessage));
            qDebug() << errorMessage;
        }

        return;
    }

    for(auto mp = mountPoints.begin(); mp != mountPoints.end(); mp++)
    {
        QProcess umount;

        QStringList args;
        for(auto it = options.cbegin(); it != options.cend(); it++)
        {
            args << it.key();
            if(!it.value().isNull())
                args << it.value().toString();
        }
        args << QString::fromLocal8Bit(*mp);

        umount.start(QStringLiteral("/sbin/umount"), args);
        umount.waitForFinished(-1);

        if(umount.exitCode() == 0)
        {
            removeMountPoint(QString::fromLocal8Bit(*mp), /*checkIfEmpty = */ true);
            mountPoints.erase(mp);
        }
        else
        {
            QString errorMessage = umount.readAllStandardError();
            conn.send(msg.createErrorReply("org.freedesktop.UDisks2.Error.Failed", errorMessage));
            qDebug() << errorMessage;
            error = true;
        }
    }

    if(!error)
        signalMountPointsChanged();
}

void BlockFilesystem::signalMountPointsChanged()
{
    QVariantMap props;
    props.insert(QStringLiteral("MountPoints"), QVariant::fromValue(mountPoints));

    QDBusMessage signal = QDBusMessage::createSignal(
                                parentBlock()->dbusPath.path(),
                                QStringLiteral("org.freedesktop.DBus.Properties"),
                                QStringLiteral("PropertiesChanged"));

    signal << QStringLiteral("org.freedesktop.UDisks2.Filesystem")
           << props
           << QStringList();
    QDBusConnection::systemBus().send(signal);
}


BlockFilesystem::~BlockFilesystem()
{
}

Block* BlockFilesystem::parentBlock()
{
    return qobject_cast<Block*>(parent());
}

BlockFilesystem::BlockFilesystem(Block* parent)
    : QObject(parent)
{
}