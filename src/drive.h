/*
    Copyright 2016-2019 Gleb Popov <6yearold@gmail.com>

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

#include <QDBusObjectPath>
#include <QObject>

#include "adaptors.h"

class Drive : public QObject,
              public QDBusContext {
    Q_OBJECT
public:
    Drive();

    QDBusObjectPath dbusPath;
    QString geomName;
    QString description;
    QString identifier;
    bool isRemovable;
    QString ataSata;

    bool camcontrolProbeDone;
    bool geomProbeDone;

    Q_PROPERTY(Configuration Configuration READ configuration)
    Configuration configuration() const;

    Q_PROPERTY(QString Vendor READ vendor)
    QString vendor() const;

    qulonglong size;
    Q_PROPERTY(qulonglong Size READ driveSize)
    qulonglong driveSize() const;

    Q_PROPERTY(QString Serial READ serial)
    QString serial() const;

    Q_PROPERTY(QStringList MediaCompatibility READ mediaCompatibility)
    QStringList mediaCompatibility() const;

    Q_PROPERTY(bool Optical READ optical)
    bool optical() const;

    Q_PROPERTY(bool Ejectable READ ejectable)
    bool ejectable() const;

    Q_PROPERTY(bool Removable READ removable)
    bool removable() const;

    Q_PROPERTY(QString ConnectionBus READ connectionBus)
    QString connectionBus() const;

    Q_PROPERTY(bool bsdisks_IsHotpluggable READ bsdisks_IsHotpluggableR)
    bool bsdisks_IsHotpluggableR() const;

    Q_PROPERTY(QString bsdisks_ConnectionBus READ bsdisks_ConnectionBusR)
    QString bsdisks_ConnectionBusR() const;

    Q_PROPERTY(QString bsdisks_AtaSata READ bsdisks_AtaSataR)
    QString bsdisks_AtaSataR() const;

public slots:
    void Eject(const QVariantMap& options);
};
