/*
 * Copyright 2020 Rafael Sadowski <rafael@sizeofvoid.org>
 * Copyright 2020-2021 Rafael Sadowski <rs@rsadowski.de>
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

#pragma once

#include "drive.h"
#include <QObject>
#include <QString>

/**
 * disklabel – read disk pack label
 */
class DiskLabel {
public:
    DiskLabel() = default;
    DiskLabel(const QString&);

    bool isValid() const;
    TDrive getDrive() const;
    QString getDeviceName() const;

private:
    void analyseDev(const QString&);
    bool isValidFileSysetem(u_int8_t) const;

    void createDrive(const QString&);
    TBlock createBlock(const QString&, const QString&, u_int64_t);
    TBlockPartition createPartition(const QString&, u_int64_t);
    TBlockFilesystem createFilesystem(const TBlock&, const QString&);

    TDrive m_drive = nullptr;
};

using TDiskLabel = std::shared_ptr<DiskLabel>;
using TDiskLabelVec = std::vector<TDiskLabel>;
