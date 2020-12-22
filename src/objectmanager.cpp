#include "objectmanager.h"
#include "adaptors.h"
#include "block.h"
#include "blockfilesystem.h"
#include "bsdisks.h"
#include "drive.h"

DBUSManagerStruct ObjectManager::GetManagedObjects()
{
    qDebug() << "GetManagedObjects";
    DBUSManagerStruct ret;

    QList<std::pair<QObject*, QDBusObjectPath>> objects;

    std::transform(m_driveObjects.begin(), m_driveObjects.end(), std::back_inserter(objects),
        [](Drive* d) { return std::make_pair(static_cast<QObject*>(d), d->dbusPath); });

    std::transform(m_blockObjects.begin(), m_blockObjects.end(), std::back_inserter(objects),
        [](Block* b) { return std::make_pair(static_cast<QObject*>(b), b->dbusPath); });

    for (auto& objectPair : objects) {
        QObject* o = objectPair.first;
        auto dbusPath = objectPair.second;

        QVariantMapMap interfaces;

        for (QObject* child : o->children()) {
            QDBusAbstractAdaptor* adaptor = qobject_cast<QDBusAbstractAdaptor*>(child);

            if (!adaptor)
                continue;

            const QString& iface = adaptor->metaObject()->classInfo(adaptor->metaObject()->indexOfClassInfo("D-Bus Interface")).value();

            QVariantMap properties;
            for (int i = adaptor->metaObject()->propertyOffset();
                 i < adaptor->metaObject()->propertyCount(); ++i) {
                auto propertyName = adaptor->metaObject()->property(i).name();
                properties.insert(QString::fromLatin1(propertyName), adaptor->property(propertyName));
            }

            interfaces[iface] = properties;
        }

        ret[dbusPath] = interfaces;
    }

    return ret;
}

void ObjectManager::initialProbe()
{
    qDebug() << "initialProbe";
    initialProbeDone = true;
}

void ObjectManager::filesystemAdded(Block* b, QString fs)
{
    qDebug() << "filesystemAdded";
    Q_ASSERT(b->bFilesystem == nullptr);

    auto bfs = new BlockFilesystem(b);
    bfs->filesystem = fs;

    b->bFilesystem = bfs;
    b->idUsage = QStringLiteral("filesystem");

    if (fs == QStringLiteral("msdosfs")) {
        b->idType = QStringLiteral("vfat");
    }
    else {
        b->idType = fs;
    }

    for (const QStorageInfo& storage : QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            if (!storage.device().compare(b->device().chopped(1))) {
                bfs->mountPoints << storage.rootPath().toLocal8Bit();
                break;
            }
        }
    }
}

void ObjectManager::addBlock(TDiskLabel const& disklabel)
{
    qDebug() << "addBlock";
    for (QString const& p : disklabel->getDevicePartitions()) {
        QString dev(disklabel->getDeviceName() + p);
        if (!m_blockObjects.contains(dev)) {
            if (auto* drive = m_driveObjects.value(dev)) {
                if (drive->vendor() == "VBOX CD-ROM" && m_blockObjects.contains(dev))
                    return;
            }

            qDebug() << "Created block " << dev;
            auto* b = new Block;
            m_blockObjects.insert(dev, b);

            b->name = dev;
            b->dbusPath = QDBusObjectPath(UDisksBlockDevices + dev);

            //startFilesystemProbe(b);
            qDebug() << "startFilesystemProbe";
            //auto prober = new FilesystemProber(b->device());
            filesystemAdded(b, QLatin1String("ffs"));
            b->labels = QStringList() << dev;
            registerBlock(b);
        }
    }
}

void ObjectManager::updateBlock(QString dev)
{
    qDebug() << "updateBlock";
    auto* b = m_blockObjects.value(dev);

    if (b->probesDone.all()) {
        b->probesDone.reset();
        //b->probesDone.set(FILESYSTEM_PROBE);
    }
    else
        b->needsAnotherProbe = true;
}

void ObjectManager::removeBlock(QString dev)
{
    qDebug() << "removeBlock";
    if (auto* drive = m_driveObjects.value(dev)) {
        if (drive->vendor() == "VBOX CD-ROM")
            return;
    }

    // happens with partitions blocks - we delete them along with table block,
    // but the devd event may arrive after that
    if (!m_blockObjects.contains(dev))
        return;

    auto* b = m_blockObjects.take(dev);
    auto* d = m_driveObjects.value(b->driveName());
    qDebug() << "Unregistering " + b->dbusPath.path();
    QStringList ifaces;

    if (b->bFilesystem)
        ifaces << QStringLiteral("org.freedesktop.UDisks2.Filesystem");
    if (b->bPartition)
        ifaces << QStringLiteral("org.freedesktop.UDisks2.Partition");
    if (b->bPartTable) {
        ifaces << QStringLiteral("org.freedesktop.UDisks2.PartitionTable");
        for (auto pb : b->bPartTable->partitionBlockNames)
            removeBlock(pb);
    }
    ifaces << QStringLiteral("org.freedesktop.UDisks2.Block");
    removeInterfaces(b->dbusPath, ifaces);
    QDBusConnection::systemBus().unregisterObject(b->dbusPath.path());

    if (!b->hasNoDrive)
        // delete drive only when we are deleting table block or any other non-partition block
        if (!b->bPartition)
            removeDrive(d->geomName);

    delete b;
}

void ObjectManager::addDrive(TDiskLabel const& dl)
{
    /*
    Q_ASSERT(!m_driveObjects.contains(dev));

    qDebug() << "Created drive " << dev;
    auto* d = new Drive;
    m_driveObjects.insert(dev, d);

    QString devPath = UDisksDrives + dev;
    qDebug() << "Created drive 2" << devPath;
    d->dbusPath = QDBusObjectPath(devPath);
    d->geomName = dev;
    */
}

void ObjectManager::removeDrive(QString dev)
{
    qDebug() << "removeDrive";
    Q_ASSERT(m_driveObjects.contains(dev));

    auto* d = m_driveObjects.take(dev);
    qDebug() << "Unregistering " + d->dbusPath.path();
    removeInterfaces(d->dbusPath, {QStringLiteral("org.freedesktop.UDisks2.Drive")});
    QDBusConnection::systemBus().unregisterObject(d->dbusPath.path());
    delete d;
}

void ObjectManager::postponeRegistration(QString blockName)
{
    qDebug() << "postponeRegistration";
    m_postponedRegistrations.insert(blockName);
}

void ObjectManager::tryRegisterPostponed()
{
    qDebug() << "tryRegisterPostponed";
    int postponedSize;
    do {
        postponedSize = m_postponedRegistrations.size();

        for (auto it = m_postponedRegistrations.begin(); it != m_postponedRegistrations.end(); it++) {
            if (registerBlock(m_blockObjects[*it], false)) {
                qDebug() << "Pop " << *it << " from m_postponedRegistrations";
                m_postponedRegistrations.erase(it);
                break;
            }
        }
    } while (postponedSize > m_postponedRegistrations.size());
}

bool ObjectManager::registerBlock(Block* b, bool tryPostponed)
{
    qDebug() << "registerBlock";
    // We've got MEDIACHANGE event while probes were running.
    // Re-start GEOM probe and postpone registration.
    if (b->needsAnotherProbe) {
        qDebug() << b->name << " changed, restarting probes ";
        b->needsAnotherProbe = false;
        //b->probesDone.reset(GEOM_PROBE);
        return false;
    }

    if (tryPostponed)
        tryRegisterPostponed();

    if (b->bPartition) {
        Block* table = m_blockObjects.value(b->bPartition->partBlockName, nullptr);
        if (!table || !table->registered) {
            qDebug() << b->name << " waits for partition table " << (table ? table->name : QString());
            postponeRegistration(b->name);
            return false;
        }
        if (!b->bPartition->partTableBlock)
            b->bPartition->partTableBlock = table;
    }

    if (!b->hasNoDrive)
        if (Drive* d = m_driveObjects.value(b->driveName(), nullptr)) {
            if (!d || !(d->camcontrolProbeDone && d->geomProbeDone)) {
                qDebug() << b->name << " waits for drive " << b->driveName();
                postponeRegistration(b->name);
                return false;
            }
        }

    if (!b->registered) {
        b->registered = true;

        QString devPath = b->dbusPath.path();

        qDebug() << "Registering " + devPath;
        QDBusConnection::systemBus().registerObject(devPath, b);

        QList<std::pair<QString, QDBusAbstractAdaptor*>> interfaces;
        interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Block"), new BlockAdaptor(b));

        if (b->bFilesystem)
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Filesystem"), new FilesystemAdaptor(b));

        if (b->bPartTable)
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.PartitionTable"), new PartitionTableAdaptor(b));

        if (b->bPartition)
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Partition"), new PartitionAdaptor(b));

        addInterfaces(b->dbusPath, interfaces);
    }
    else {
    } // we were updating things after updateBlock()

    if (tryPostponed)
        tryRegisterPostponed();

    return true;
}

void ObjectManager::registerDrive(Drive* d)
{
    qDebug() << "registerDrive";
    const QString dbusPath = d->dbusPath.path();

    qDebug() << "Registering " + dbusPath;
    QDBusConnection::systemBus().registerObject(dbusPath, d);

    addInterfaces(d->dbusPath, {std::make_pair("org.freedesktop.UDisks2.Drive", new DriveAdaptor(d))});

    tryRegisterPostponed();
}

void ObjectManager::startFilesystemProbe(Block* b)
{
    qDebug() << "startFilesystemProbe";
    const QString devName = b->name;
    filesystemAdded(b, QLatin1String("ffs"));
    b->labels = QStringList() << devName;
    registerBlock(b);
}

void ObjectManager::addPartition(Block* b, const QString& tableBlockName)
{
    qDebug() << "addPartition";
    auto* bp = new BlockPartition(b);

    bp->partBlockName = tableBlockName;
    bp->size = 133333;
    /*
    bp->partitionType = partInfo.type();
    bp->number = partInfo.index();
    bp->offset = partInfo.offset();
    bp->size = partInfo.length();
    */

    b->bPartition = bp;
}

void ObjectManager::addInterfaces(QDBusObjectPath path, QList<std::pair<QString, QDBusAbstractAdaptor*>> newInterfaces)
{
    qDebug() << "addInterfaces";
    QVariantMapMap interfaces;

    for (auto pair : newInterfaces) {
        const QString& iface = pair.first;
        QDBusAbstractAdaptor* adaptor = pair.second;

        QVariantMap properties;
        for (int i = adaptor->metaObject()->propertyOffset();
             i < adaptor->metaObject()->propertyCount(); ++i) {
            auto propertyName = adaptor->metaObject()->property(i).name();
            properties.insert(QString::fromLatin1(propertyName), adaptor->property(propertyName));
        }

        interfaces[iface] = properties;
    }

    emit InterfacesAdded(path, interfaces);
}

void ObjectManager::removeInterfaces(QDBusObjectPath path, QStringList ifaces)
{
    qDebug() << "removeInterfaces";
    emit InterfacesRemoved(path, ifaces);
}
