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

#pragma once

#include <sys/types.h>

#include <QObject>
#include <QSharedData>
#include <QRunnable>

class DiskInfo
{
    friend class GeomProber;
public:
    DiskInfo() : d(new DiskInfoD) {};

    QString devName() const { return d->devName; }
    QString descr() const { return d->descr; }
    QString ident() const { return d->ident; }
    off_t mediaSize() const { return d->mediaSize; }
    u_int sectorSize() const { return d->sectorSize; }
    u_int sectors() const { return d->sectors; }
    u_int heads() const { return d->heads; }
private:
    struct DiskInfoD : public QSharedData
    {
        QString devName;
        QString descr;
        QString ident;
        off_t mediaSize;
        u_int sectorSize;
        u_int sectors;
        u_int heads;
    };
    QSharedDataPointer<DiskInfoD> d;
};
Q_DECLARE_METATYPE(DiskInfo)

class Part
{
    friend class GeomProber;
public:
    Part() : d(new PartD) {};

    QString type() const { return d->type; }
    long long unsigned int length() const { return d->length; }
    u_int offset() const { return d->offset; }
    u_char rawtype() const { return d->rawtype; }
    u_char index() const { return d->index; }
private:
    struct PartD : public QSharedData
    {
        QString type;
        long long unsigned int length;
        u_int offset;
        u_char rawtype;
        u_char index;
    };
    QSharedDataPointer<PartD> d;
};
Q_DECLARE_METATYPE(Part)

class PartTableInfo
{
    friend class GeomProber;
public:
    PartTableInfo() : d(new PartInfoD) {};

    QString scheme() const { return d->scheme; }
    QHash<QString, Part> partitions() const { return d->partitions; }
private:
    struct PartInfoD : public QSharedData
    {
        QString scheme;
        QHash<QString, Part> partitions;
    };
    QSharedDataPointer<PartInfoD> d;
};
Q_DECLARE_METATYPE(PartTableInfo)

class GeomProber : public QObject, public QRunnable
{
    Q_OBJECT
public:
    GeomProber(QString dev = QString()) : m_dev(dev) {}
    virtual ~GeomProber() {}
    virtual void run() override;

signals:
    void gotLabels(QStringList);
    void gotDisk(DiskInfo);
    void gotPartTable(PartTableInfo);
    void gotPart(QString tableBlockName, Part partInfo);
    void finished();

private:
    QString m_dev;
};
