/*
    Copyright 2016 Gleb Popov <6yearold@gmail.com>
    Copyright 2020-2021 Rafael Sadowski <rs@rsadowski.de>

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

#include "adaptors.h"
#include "block.h"
#include "bsdisks.h"
#include "disk_thread.h"
#include "drive.h"
#include "manageradaptor.h"
#include "objectmanager.h"

#include <err.h>
#include <iostream>
#include <syslog.h>
#include <unistd.h>

#include <QSet>

ObjectManager manager;

/* Initialized in main */
static bool syslog_output;
static bool debug;
static bool verbose;

static void msg_handler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    auto printMsg = [&](int priority, const char* state, const char* message) {
        if (syslog_output) {
            syslog(LOG_DEBUG | LOG_DAEMON, "%s\n", message);
        } else {
            if (verbose) {
                const char* file = context.file ? context.file : "";
                std::clog << state << ": " << message << "(" << file << ":" << context.line << ")"
                          << "\n";
            } else {
                std::clog << state << ": " << message << "\n";
            }
        }
    };

    switch (type) {
    case QtDebugMsg:
        if (debug)
            printMsg(LOG_DEBUG, "Debug", msg.toLocal8Bit());
        break;
    case QtInfoMsg:
        printMsg(LOG_INFO, "Info", msg.toLocal8Bit());
        break;
    case QtWarningMsg:
        printMsg(LOG_WARNING, "Warning", msg.toLocal8Bit());
        break;
    case QtCriticalMsg:
        printMsg(LOG_ERR, "Critical", msg.toLocal8Bit());
        break;
    case QtFatalMsg:
        printMsg(LOG_CRIT, "Fatal", msg.toLocal8Bit());
        break;
    }
}

int main(int argc, char** argv)
{
    if (unveil("/", "rwc") == -1)
        err(1, "unveil /");
    if (unveil("/sbin/umount", "rx") == -1)
        err(1, "unveil /sbin/umount");
    if (unveil("/sbin/mount_ffs", "rx") == -1)
        err(1, "unveil /sbin/mount_ffs");
    if (unveil("/sbin/mount_ext2fs", "rx") == -1)
        err(1, "unveil /sbin/mount_ext2fs");
    if (unveil("/sbin/mount_ntfs", "rx") == -1)
        err(1, "unveil /sbin/mount_ntfs");
    if (unveil("/sbin/mount_msdos", "rx") == -1)
        err(1, "unveil /sbin/mount_msdos");
    if (unveil("/sbin/mount_cd9660", "rx") == -1)
        err(1, "unveil /sbin/mount_cd9660");
    if (unveil(NULL, NULL) == -1)
        err(1, "unveil NULL");

    qInstallMessageHandler(msg_handler);

    qRegisterMetaType<Configuration>();
    qRegisterMetaType<ConfigurationList>();
    qRegisterMetaType<QVariantMapMap>();
    qRegisterMetaType<DBUSManagerStruct>();
    qRegisterMetaType<QSet<QString>>();

    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<Configuration>();
    qDBusRegisterMetaType<ConfigurationList>();
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<DBUSManagerStruct>();

    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication app(argc, argv);

    if (!QDBusConnection::systemBus().registerService("org.freedesktop.UDisks2")) {
        qCritical() << "Could not register UDisks2 service";
        return -1;
    }

    syslog_output = QCoreApplication::arguments().contains("--syslog");
#if defined(__OpenBSD__)
    if (syslog_output)
        setprogname("openbsdisks2");
#endif

    debug = QCoreApplication::arguments().contains("--debug") ||
        QCoreApplication::arguments().contains("-d");

    verbose = QCoreApplication::arguments().contains("--verbose") ||
        QCoreApplication::arguments().contains("-v");

    new ObjectManagerAdaptor(&manager);

    QThreadPool::globalInstance()->setExpiryTimeout(-1);
    QThreadPool::globalInstance()->setMaxThreadCount(4);

    QDBusConnection::systemBus().registerObject("/org/freedesktop/UDisks2", &manager);

    DiskThread disk;
    QObject::connect(&disk, &DiskThread::deviceAdded, &manager, &ObjectManager::addDrive);

    QObject::connect(&disk, &DiskThread::blockAdded, &manager, &ObjectManager::addBlock);

    QObject::connect(&disk, &DiskThread::deviceRemoved, &manager, &ObjectManager::removeDrive);

    disk.start();
    app.exec();
}
