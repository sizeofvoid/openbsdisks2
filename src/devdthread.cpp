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

#include "devdthread.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHash>
#include <QRegularExpression>

#include <errno.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

void DevdThread::parseLine(QString line)
{
    // !system=DEVFS subsystem=CDEV type=CREATE cdev=da0
    auto message = line.split(' ');
    QHash<QString, QString> data;
    Q_FOREACH (auto m, message) {
        auto mm = m.split('=');

        //skip things like "on", "at", etc.
        // +umass0 at bus=0 hubaddr=1 port=2 <...> intprotocol=0x50 on uhub0
        if (mm.count() == 1)
            continue;

        data.insert(mm[0], mm[1]);
    }

    QString devArg = data["cdev"].trimmed();

    // don't do anything with stuff like diskid/blabla
    if (devArg.contains('/'))
        return;

    if (QCoreApplication::arguments().contains("--debug-devd"))
        qDebug() << "devd message: " << data;

    // !system=GEOM subsystem=DEV type=CREATE cdev=da0
    // GEOM messages are always appear after corresponding DEVFS ones, so
    // we are looking for them only
    if (data["!system"] == "GEOM" && data["subsystem"] == "DEV") {
        if (data["type"] == "CREATE") {
            emit blockAdded(devArg);

            // there are no subsystem=disk events for MMC, try to workaround it
            QRegularExpression re("mmcsd\\d$");
            if (re.match(devArg).hasMatch())
                emit driveAdded(devArg);
        }
        if (data["type"] == "DESTROY")
            emit blockRemoved(devArg);
        if (data["type"] == "MEDIACHANGE")
            emit blockChanged(devArg);

        return;
    }

    // !system=GEOM subsystem=disk type=GEOM::physpath devname=da0
    // there is no devd event for drive removal
    if (data["!system"] == "GEOM" && data["subsystem"] == "disk" && data["type"] == "GEOM::physpath") {
        emit driveAdded(data["devname"].trimmed());
        return;
    }
}

void DevdThread::run()
{
    struct sockaddr_un addr;
    int res;
    int s = socket(PF_LOCAL, SOCK_SEQPACKET, 0);

    addr.sun_family = PF_LOCAL;
    strcpy(addr.sun_path, "/var/run/devd.seqpacket.pipe");
    SUN_LEN(&addr);

    res = ::connect(s, (struct sockaddr*)&addr, sizeof(addr));
    if (res == -1) {
        printf("Unable to connect to /var/run/devd.seqpacket.pipe: %s\n", strerror(errno));
        return;
    }

    while (true) {
        char buf[2048];
        res = recv(s, buf, 2048, 0);
        if (res == -1) {
            printf("Unable to recv() from /var/run/devd.seqpacket.pipe: %s\n", strerror(errno));
            return;
        }

        buf[res] = 0;
        parseLine(QString::fromLatin1(buf));
    }
}
