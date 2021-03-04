/*
    Copyright 2016 Gleb Popov <6yearold@gmail.com>
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

#include <QSet>

#include <stdarg.h>
#include <sysexits.h>
#include <syslog.h>

#include "adaptors.h"
#include "block.h"
#include "bsdisks.h"
#include "disk_thread.h"
#include "drive.h"
#include "manageradaptor.h"
#include "objectmanager.h"

ObjectManager manager;
BsdisksConfig config;

BsdisksConfig& BsdisksConfig::get()
{
    return config;
}

/* Initialized in main */
static QtMessageHandler old_message_handler = NULL;
static bool syslog_output;
static bool no_debug;

static void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (syslog_output) {
        QByteArray local_msg = msg.toLocal8Bit();
        switch (type) {
        case QtDebugMsg:
            syslog(LOG_DEBUG | LOG_DAEMON, "%s\n", local_msg.constData());
            break;
        case QtInfoMsg:
            syslog(LOG_INFO | LOG_DAEMON, "%s\n", local_msg.constData());
            break;
        case QtWarningMsg:
            syslog(LOG_WARNING | LOG_DAEMON, "%s\n", local_msg.constData());
            break;
        case QtCriticalMsg:
            /* Tone it down a bit here */
            syslog(LOG_ERR | LOG_DAEMON, "%s\n", local_msg.constData());
            break;
        case QtFatalMsg:
            /* Tone it down a bit here */
            syslog(LOG_CRIT | LOG_DAEMON, "%s\n", local_msg.constData());
            break;
        }
    }

    if ((old_message_handler != NULL) && !no_debug)
        (*old_message_handler)(type, context, msg);
}

int main(int argc, char** argv)
{
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

    QFile configFile("/etc/bsdisks.conf");
    configFile.open(QIODevice::ReadOnly);

    if (configFile.isOpen()) {
        QByteArray line;
        do {
            line = configFile.readLine(1024);
            QString statement = QString::fromLocal8Bit(line).trimmed();
            QStringList parsed = statement.split('=');

            if (parsed.size() < 2)
                continue;

            if (parsed[0].trimmed().startsWith('#'))
                continue;

            if (parsed[0].trimmed() == QStringLiteral("mount_msdosfs_flags"))
                BsdisksConfig::get().MountMsdosfsFlags = parsed[1].trimmed();
        } while (!line.isEmpty());
    }
    configFile.close();

    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication app(argc, argv);

    if (!QDBusConnection::systemBus().registerService("org.freedesktop.UDisks2")) {
        qCritical() << "Could not register UDisks2 service";
        return EX_UNAVAILABLE;
    }

    syslog_output = QCoreApplication::arguments().contains("--syslog-output");
    no_debug = QCoreApplication::arguments().contains("--no-debug");
    old_message_handler = qInstallMessageHandler(&message_handler);

    new ObjectManagerAdaptor(&manager);

    QThreadPool::globalInstance()->setExpiryTimeout(-1);
    QThreadPool::globalInstance()->setMaxThreadCount(4);

    manager.initialProbe();

    QDBusConnection::systemBus().registerObject(
        "/org/freedesktop/UDisks2", &manager);

    DiskThread disk;
    QObject::connect(&disk, &DiskThread::deviceAdded,
        &manager, &ObjectManager::addDrive);
    QObject::connect(&disk, &DiskThread::blockAdded,
        &manager, &ObjectManager::addBlock);

    QObject::connect(&disk, &DiskThread::blockRemoved,
        &manager, &ObjectManager::removeBlock);

    disk.start();

    app.exec();
}
