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
#include "block.h"
#include "blockpartition.h"
#include "bsdisks.h"

#define DKTYPENAMES
#include <sys/disklabel.h>

#include <fcntl.h>
#include <sys/dkio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <util.h>

#include <QChar>
#include <QDebug>
#include <QString>
#include <QUuid>

DiskLabel::DiskLabel(const QString& dev)
{
    DiskLabel::analyseDev(dev);
}

void DiskLabel::analyseDev(const QString& dev)
{
    struct disklabel lab;
    char* specname;
    int f = opendev(dev.toLocal8Bit().data(), O_RDONLY, OPENDEV_PART, &specname);

    if (ioctl(f, DIOCGDINFO, &lab) == -1) {
        close(f);
        return;
    }

    const u_int64_t blockSize = DL_GETDSIZE(&lab);

    createDrive(dev);
    const QUuid duid(0x0,
                     0x0,
                     0x0,
                     lab.d_uid[0],
                     lab.d_uid[1],
                     lab.d_uid[2],
                     lab.d_uid[3],
                     lab.d_uid[4],
                     lab.d_uid[5],
                     lab.d_uid[6],
                     lab.d_uid[7]);
    m_drive->setId(QString(specname).replace("/dev/", "dev_"));
    m_drive->setDuid(duid);
    m_drive->setVendor(QString(lab.d_packname));
    m_drive->setSize(blockSize);

    const QString sduid = QString::asprintf("%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
                                            lab.d_uid[0],
                                            lab.d_uid[1],
                                            lab.d_uid[2],
                                            lab.d_uid[3],
                                            lab.d_uid[4],
                                            lab.d_uid[5],
                                            lab.d_uid[6],
                                            lab.d_uid[7]);

    struct disklabel::partition* pp = nullptr;

    for (int i = 0; i < lab.d_npartitions; i++) {
        pp = &lab.d_partitions[i];
        if (DL_GETPSIZE(pp) > 0) {
            const QString p = QChar('a' + i);
            if (p != QStringLiteral("c")) {
                if (isValidFileSysetem(pp->p_fstype)) {
                    auto block = createBlock(
                        getDeviceName() + p, QString(fstypesnames[pp->p_fstype]), blockSize);
                    block->setId(sduid + p);
                    block->setIdLabel(lab.d_packname);
                    auto fs = createFilesystem(block, QString(fstypesnames[pp->p_fstype]));
                    auto partition = createPartition(p, DL_GETPSIZE(pp));

                    partition->addFilesystem(fs);
                    block->addPartition(partition);
                    m_drive->addBlock(block);
                }
            }
        }
    }
    close(f);
}

bool DiskLabel::isValidFileSysetem(u_int8_t fstype) const
{
    switch (fstype) {
    case FS_UNUSED:
    case FS_SWAP:
    case FS_V6:
    case FS_V7:
    case FS_SYSV:
    case FS_V71K:
    case FS_V8:
        return false;

    case FS_BSDFFS:
    case FS_MSDOS:
        return true;

    case FS_BSDLFS:
    case FS_OTHER:
    case FS_HPFS:
        return false;
    case FS_ISO9660:
        return true;

    case FS_BOOT:
    case FS_ADOS:
    case FS_HFS:
    case FS_ADFS:
        return false;

    case FS_EXT2FS:
        return true;

    case FS_CCD:
    case FS_RAID:
        return false;

    case FS_NTFS:
        return true;

    case FS_UDF:
        return false;
    };
    return false;
}

TDrive DiskLabel::getDrive() const
{
    return m_drive;
}

QString DiskLabel::getDeviceName() const
{
    assert(getDrive());
    return getDrive() ? getDrive()->getDeviceName() : QString();
}

bool DiskLabel::isValid() const
{
    return m_drive != nullptr;
}

void DiskLabel::createDrive(const QString& dev)
{
    m_drive = std::make_shared<Drive>(dev);
}

TBlock DiskLabel::createBlock(const QString& dev, const QString& fstype, u_int64_t blockSize)
{
    auto block = std::make_shared<Block>(dev);

    block->setSize(blockSize);
    block->setIdUsage(QStringLiteral("filesystem"));
    block->setIdType(fstype);

    return block;
}

TBlockPartition DiskLabel::createPartition(const QString& partitionNumber, u_int64_t partitionSize)
{
    auto partition = std::make_shared<BlockPartition>(partitionNumber);
    partition->size = partitionSize;
    return partition;
}

TBlockFilesystem DiskLabel::createFilesystem(const TBlock& block, const QString& fstype)
{
    auto bfs = std::make_shared<BlockFilesystem>();
    bfs->setFilesystem(fstype);

    const auto volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& storage : volumes) {
        if (storage.isValid() && storage.isReady()) {
            if (!storage.device().compare(block->device().chopped(1))) {
                bfs->addMountPoint(storage.rootPath());
                break;
            }
        }
    }
    return bfs;
}
