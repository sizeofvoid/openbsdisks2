/*
    Copyright 2020 Gleb Popov <6yearold@gmail.com>

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

#include "zfsprober.h"

#include <sys/param.h>
#include <sys/module.h>

#include <QDebug>
#include <QRegularExpression>
#include <QProcess>

void ZFSProber::run()
{
    if(::modfind("zfs") == -1)
    {
        qDebug() << "Not probing ZFS, the kernel module isn't loaded";
        return;
    }

    QProcess zfsProcess;
    QStringList args;
    args << QStringLiteral("get") << QStringLiteral("-H") << QStringLiteral("canmount");

    zfsProcess.start(QStringLiteral("/sbin/zfs"), args);

    bool res = zfsProcess.waitForFinished(-1);
    Q_ASSERT(res);

    int exitCode = zfsProcess.exitCode();
    if(exitCode)
        return;

    QString output = zfsProcess.readAllStandardOutput().trimmed();
    if(output.isEmpty())
        return;

    // prepare for "zfs get mounted,mountpoint" call
    args.clear();
    args << QStringLiteral("get") << QStringLiteral("-H") << QStringLiteral("mounted,mountpoint");

    Q_FOREACH(auto line, output.split('\n'))
    {
        // ztank/home      canmount        on      default
        // ztank/kderoot   canmount        noauto  local
        auto parsed = line.splitRef(QRegularExpression("\\s+"));

        auto dataset = parsed[0];
        auto canmountVal = parsed[2];

        if(canmountVal == QStringLiteral("noauto"))
        {
            args << dataset.toString();
            zfsProcess.start(QStringLiteral("/sbin/zfs"), args);

            res = zfsProcess.waitForFinished(-1);
            Q_ASSERT(res);

            exitCode = zfsProcess.exitCode();
            if(exitCode)
                return;

            // ztank/kderoot   mounted no      -
            // ztank/kderoot   mountpoint      /home/arrowd/kderoot      local
            QString output2 = zfsProcess.readAllStandardOutput().trimmed();
            auto lines = output2.split('\n');

            bool mounted = lines[0].splitRef(QRegularExpression("\\s+"))[2] == QStringLiteral("yes");
            QString mountpoint = lines[1].splitRef(QRegularExpression("\\s+"))[2].toString();

            args.pop_back();

            // we aren't interested in root FS
            if(mountpoint != '/')
            {
                ZFSInfo info;
                info.d->dataset = dataset.toString();
                info.d->mountpoint = mountpoint;
                info.d->isMounted = mounted;

                emit gotDataset(info);
            }
        }
    }
}
