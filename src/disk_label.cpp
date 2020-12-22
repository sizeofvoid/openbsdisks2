/*
 * Copyright 2020 Rafael Sadowski <rafael@sizeofvoid.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "disk_label.h"
#include <QChar>
#include <QString>

#include <QDebug>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/disklabel.h>
#include <sys/dkio.h>
#include <sys/ioctl.h>
#include <util.h>
#include <unistd.h>

DiskLabel::DiskLabel(const QString& dev)
    : m_deviceName(dev)
{
    analyseDev();
}

void DiskLabel::analyseDev()
{
    struct disklabel lab;
    char* specname;
    int f = opendev(m_deviceName.toLocal8Bit().data(),
        O_RDONLY,
        OPENDEV_PART,
        &specname);


    if (ioctl(f, DIOCGDINFO, &lab) == -1) {
        close(f);
        return;
        //err(4, "DIOCRLDINFO");
    }

    struct disklabel::partition* pp = nullptr;

    for (int i = 0; i < lab.d_npartitions; i++) {
        pp = &lab.d_partitions[i];
        double p_size;

        if (DL_GETPSIZE(pp)) {
            u_int32_t frag = DISKLABELV1_FFS_FRAG(pp->p_fragblock);
            u_int32_t fsize = DISKLABELV1_FFS_FSIZE(pp->p_fragblock);
            QChar p('a' + i);
            if (p != QChar('c'))
                m_devicePartitions << p;
        }
    }
    close(f);
}

QStringList const&
DiskLabel::getDevicePartitions() const
{
    return m_devicePartitions;
}

QString const&
DiskLabel::getDeviceName() const
{
    return m_deviceName;
}

bool
DiskLabel::isValid() const
{
    return !m_devicePartitions.isEmpty();
}
