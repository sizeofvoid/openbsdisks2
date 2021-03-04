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

#include <QHash>

#include "disk_label.h"
#include "manageradaptor.h"

class Block;
class Drive;

class ObjectManager : public QObject {
    Q_OBJECT
public slots:
    DBUSManagerStruct GetManagedObjects();

    void filesystemAdded(Block* b, QString fs);

    void addBlock(TDiskLabel const&);
    void updateBlock(QString dev);
    void removeBlock(TDiskLabel const&);

    void addDrive(TDiskLabel const&);
    void removeDrive(TDiskLabel const&);

    void initialProbe();
signals:
    void InterfacesAdded(const QDBusObjectPath& object_path, const QVariantMapMap& interfaces_and_properties);
    void InterfacesRemoved(const QDBusObjectPath& object_path, const QStringList& interfaces);

private:
    void startFilesystemProbe(Block* b);

    void registerDrive(Drive* d);
    bool registerBlock(Block* d, bool tryPostponed = true);
    void postponeRegistration(QString blockName);
    void tryRegisterPostponed();

    void addPartition(Block* b, const QString& tableBlockName);

    void addInterfaces(QDBusObjectPath path, QList<std::pair<QString, QDBusAbstractAdaptor*>> newInterfaces);
    void removeInterfaces(QDBusObjectPath path, QStringList ifaces);

    bool initialProbeDone;
    QHash<QString, Block*> m_blockObjects;
    QHash<QString, Drive*> m_driveObjects;
    QSet<QString> m_postponedRegistrations;
};
