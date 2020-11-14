#include "bsdisks.h"
#include "adaptors.h"
#include "objectmanager.h"
#include "block.h"
#include "blockfilesystem.h"
#include "drive.h"
#include "filesystemprober.h"
#include "camcontrolprober.h"
#include "zfsprober.h"

DBUSManagerStruct ObjectManager::GetManagedObjects()
{
    DBUSManagerStruct ret;

    QList<std::pair<QObject*, QDBusObjectPath>> objects;

    std::transform(m_driveObjects.begin(), m_driveObjects.end(), std::back_inserter(objects),
        [](Drive* d) { return std::make_pair(static_cast<QObject*>(d), d->dbusPath); }
    );

    std::transform(m_blockObjects.begin(), m_blockObjects.end(), std::back_inserter(objects),
        [](Block* b) { return std::make_pair(static_cast<QObject*>(b), b->dbusPath); }
    );

    for(auto& objectPair : objects)
    {
        QObject* o = objectPair.first;
        auto dbusPath = objectPair.second;

        QVariantMapMap interfaces;

        for(QObject* child : o->children())
        {
            QDBusAbstractAdaptor* adaptor = qobject_cast<QDBusAbstractAdaptor*>(child);

            if(!adaptor)
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
    auto* geomProber = new GeomProber;

    QObject::connect(geomProber, &GeomProber::gotDisk, this,
        [this](DiskInfo di)
        {
            addDrive(di.devName());

            // assume that any object from `geom disk list` has a block device in /dev
            addBlock(di.devName());
        });
    QObject::connect(geomProber, &GeomProber::gotPartTable, this,
        [this](PartTableInfo pi)
        {
            foreach(auto devName, pi.partitions().keys())
                addBlock(devName);
        });

    QThreadPool::globalInstance()->start(geomProber);

    auto* zfsProber = new ZFSProber();
    QObject::connect(zfsProber, &ZFSProber::gotDataset, this, &ObjectManager::addZFSDataset);

    QThreadPool::globalInstance()->start(zfsProber);

    initialProbeDone = true;
}

void ObjectManager::filesystemAdded(Block* b, QString fs, const ZFSInfo* zfsInfo)
{
    Q_ASSERT(b->bFilesystem == nullptr);

    auto bfs = new BlockFilesystem(b);
    bfs->filesystem = fs;

    b->bFilesystem = bfs;
    b->idUsage = QStringLiteral("filesystem");

    if(fs == QStringLiteral("msdosfs"))
    {
        b->idType = QStringLiteral("vfat");
    }
    else
    {
        b->idType = fs;
    }

    if(fs == QStringLiteral("zfs"))
    {
        bfs->zfsDataset = zfsInfo->dataSet();
        bfs->zfsMountpoint = zfsInfo->mountPoint();
        if(zfsInfo->isMounted())
            bfs->mountPoints << zfsInfo->mountPoint().toLocal8Bit();

        b->hasNoDrive = true;
    }
    else
    {
        for (const QStorageInfo &storage : QStorageInfo::mountedVolumes())
        {
            if(storage.isValid() && storage.isReady())
            {
                if(!storage.device().compare(b->device().chopped(1)))
                {
                    bfs->mountPoints << storage.rootPath().toLocal8Bit();
                    break;
                }
            }
        }
    }
}

void ObjectManager::addBlock(QString dev)
{
    if(auto* drive = m_driveObjects.value(dev))
    {
        if(drive->vendor() == "VBOX CD-ROM" && m_blockObjects.contains(dev))
            return;
    }

    Q_ASSERT(!m_blockObjects.contains(dev));

    qDebug() << "Created block " << dev;
    auto* b = new Block;
    m_blockObjects.insert(dev, b);

    b->name = dev;
    b->dbusPath = QDBusObjectPath(UDisksBlockDevices + dev);

    startFilesystemProbe(b);
    startGeomProbe(b);
}

void ObjectManager::updateBlock(QString dev)
{
    auto* b = m_blockObjects.value(dev);

    if(b->probesDone.all())
    {
        b->probesDone.reset();
        b->probesDone.set(FILESYSTEM_PROBE);
        startGeomProbe(b);
    }
    else
        b->needsAnotherProbe = true;
}


void ObjectManager::removeBlock(QString dev)
{
    if(auto* drive = m_driveObjects.value(dev))
    {
        if(drive->vendor() == "VBOX CD-ROM")
            return;
    }

    // happens with partitions blocks - we delete them along with table block,
    // but the devd event may arrive after that
    if(!m_blockObjects.contains(dev))
        return;

    auto* b = m_blockObjects.take(dev);
    auto* d = m_driveObjects.value(b->driveName());
    qDebug() << "Unregistering " + b->dbusPath.path();
    QStringList ifaces;

    if(b->bFilesystem)
        ifaces << QStringLiteral("org.freedesktop.UDisks2.Filesystem");
    if(b->bPartition)
        ifaces << QStringLiteral("org.freedesktop.UDisks2.Partition");
    if(b->bPartTable)
    {
        ifaces << QStringLiteral("org.freedesktop.UDisks2.PartitionTable");
        for(auto pb : b->bPartTable->partitionBlockNames)
            removeBlock(pb);
    }
    ifaces << QStringLiteral("org.freedesktop.UDisks2.Block");
    removeInterfaces(b->dbusPath, ifaces);
    QDBusConnection::systemBus().unregisterObject(b->dbusPath.path());

    if(!b->hasNoDrive)
        // delete drive only when we are deleting table block or any other non-partition block
        if(!b->bPartition)
            removeDrive(d->geomName);

    delete b;
}

void ObjectManager::addDrive(QString dev)
{
    Q_ASSERT(!m_driveObjects.contains(dev));

    qDebug() << "Created drive " << dev;
    auto* d = new Drive;
    m_driveObjects.insert(dev, d);

    QString devPath = UDisksDrives + dev;
    d->dbusPath = QDBusObjectPath(devPath);
    d->geomName = dev;

    auto geomProber = new GeomProber(d->geomName);
    QObject::connect(geomProber, &GeomProber::gotDisk, this,
                     [this, d](DiskInfo di)
    {
        d->size = di.mediaSize();
        d->description = di.descr();
        d->identifier = di.ident();

        d->geomProbeDone = true;
        if(d->camcontrolProbeDone)
            registerDrive(d);
    });

    auto camControlProber = new CamControlProber(d->geomName);
    QObject::connect(camControlProber, &CamControlProber::finished, this,
                     [this, d](bool isRemovable, int transport, bool isAtaSata)
    {
        d->isRemovable = isRemovable;
        d->ataSata = isAtaSata ? QStringLiteral("sata") : QStringLiteral("ata");
        d->transport = static_cast<camtransport>(transport);
        d->camcontrolProbeDone = true;
        if(d->geomProbeDone)
            registerDrive(d);
    });

    QThreadPool::globalInstance()->start(geomProber);
    QThreadPool::globalInstance()->start(camControlProber);
}

void ObjectManager::removeDrive(QString dev)
{
    Q_ASSERT(m_driveObjects.contains(dev));

    auto* d = m_driveObjects.take(dev);
    qDebug() << "Unregistering " + d->dbusPath.path();
    removeInterfaces(d->dbusPath, { QStringLiteral("org.freedesktop.UDisks2.Drive") });
    QDBusConnection::systemBus().unregisterObject(d->dbusPath.path());
    delete d;
}

void ObjectManager::postponeRegistration(QString blockName)
{
    m_postponedRegistrations.insert(blockName);
}

void ObjectManager::tryRegisterPostponed()
{
    int postponedSize;
    do
    {
        postponedSize = m_postponedRegistrations.size();

        for(auto it = m_postponedRegistrations.begin(); it != m_postponedRegistrations.end(); it++)
        {
            if(registerBlock(m_blockObjects[*it], false))
            {
                qDebug() << "Pop " << *it << " from m_postponedRegistrations";
                m_postponedRegistrations.erase(it);
                break;
            }
        }
    } while(postponedSize > m_postponedRegistrations.size());
}

bool ObjectManager::registerBlock(Block* b, bool tryPostponed)
{
    // We've got MEDIACHANGE event while probes were running.
    // Re-start GEOM probe and postpone registration.
    if(b->needsAnotherProbe)
    {
        qDebug() << b->name << " changed, restarting probes ";
        b->needsAnotherProbe = false;
        b->probesDone.reset(GEOM_PROBE);
        startGeomProbe(b);
        return false;
    }

    if(tryPostponed)
        tryRegisterPostponed();

    if(b->bPartition)
    {
        Block* table = m_blockObjects.value(b->bPartition->partBlockName, nullptr);
        if(!table || !table->registered)
        {
            qDebug() << b->name << " waits for partition table " << (table ? table->name : QString());
            postponeRegistration(b->name);
            return false;
        }
        if(!b->bPartition->partTableBlock)
            b->bPartition->partTableBlock = table;
    }

    if(!b->hasNoDrive)
        if(Drive* d = m_driveObjects.value(b->driveName(), nullptr))
        {
            if(!d || !(d->camcontrolProbeDone && d->geomProbeDone))
            {
                qDebug() << b->name << " waits for drive " << b->driveName();
                postponeRegistration(b->name);
                return false;
            }
        }

    if(!b->registered)
    {
        b->registered = true;

        QString devPath = b->dbusPath.path();

        qDebug() << "Registering " + devPath;
        QDBusConnection::systemBus().registerObject(devPath, b);

        QList<std::pair<QString, QDBusAbstractAdaptor*>> interfaces;
        interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Block"), new BlockAdaptor(b));

        if(b->bFilesystem)
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Filesystem"), new FilesystemAdaptor(b));

        if(b->bPartTable)
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.PartitionTable"), new PartitionTableAdaptor(b));

        if(b->bPartition)
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Partition"), new PartitionAdaptor(b));

        addInterfaces(b->dbusPath, interfaces);
    }
    else {} // we were updating things after updateBlock()

    if(tryPostponed)
        tryRegisterPostponed();

    return true;
}


void ObjectManager::registerDrive(Drive* d)
{
    const QString dbusPath = d->dbusPath.path();

    qDebug() << "Registering " + dbusPath;
    QDBusConnection::systemBus().registerObject(dbusPath, d);

    addInterfaces(d->dbusPath, {std::make_pair("org.freedesktop.UDisks2.Drive", new DriveAdaptor(d))});

    tryRegisterPostponed();
}

void ObjectManager::addZFSDataset(const ZFSInfo& zfsInfo)
{
    QString normalizedDatasetName = zfsInfo.dataSet().replace('/', '_'); // turn pool/foo into pool_foo
    Q_ASSERT(!m_blockObjects.contains(normalizedDatasetName));

    auto* b = new Block;
    m_blockObjects.insert(normalizedDatasetName, b);

    QString devPath = UDisksBlockDevices + normalizedDatasetName;
    b->name = normalizedDatasetName;
    b->dbusPath = QDBusObjectPath(devPath);

    filesystemAdded(b, QStringLiteral("zfs"), &zfsInfo);

    registerBlock(b);
}


void ObjectManager::startFilesystemProbe(Block* b)
{
    auto prober = new FilesystemProber(b->device());
    const QString devName = b->name;

    QObject::connect(prober, &FilesystemProber::finished, this,
                     [this, devName](QString fs)
    {
        Block* b = m_blockObjects.value(devName);
        if(!b)
            return;

        qDebug() << "Finished FS probe on " << b->name;
        if(!fs.isEmpty())
            filesystemAdded(b, fs);

        b->probesDone.set(FILESYSTEM_PROBE);
        if(b->probesDone.all())
            registerBlock(b);
    });

    QThreadPool::globalInstance()->start(prober);
}


void ObjectManager::startGeomProbe(Block* b)
{
    auto prober = new GeomProber(b->name);

    if(b->name.startsWith("md"))
        b->hasNoDrive = true;

    const QString devName = b->name;

    QObject::connect(prober, &GeomProber::gotLabels, this,
                     [this, devName](QStringList labels)
    {
        Block* b = m_blockObjects.value(devName);
        if(!b)
            return;

        b->labels = labels;
    });
    QObject::connect(prober, &GeomProber::gotDisk, this,
                     [this, devName](DiskInfo di)
    {
        Block* b = m_blockObjects.value(devName);
        if(!b)
            return;

        b->size = di.mediaSize();
        b->description = di.descr();
        b->identifier = di.ident();
    });
    QObject::connect(prober, &GeomProber::gotPartTable, this,
                     [this, devName](PartTableInfo pi)
    {
        Block* b = m_blockObjects.value(devName);
        if(!b)
            return;

        // do not create BlockPartTable when called from updateBlock()
        if(!b->bPartTable)
            b->bPartTable = new BlockPartTable(b);

        b->bPartTable->setTableType(pi.scheme());

        for(auto it = pi.partitions().constBegin(); it != pi.partitions().cend(); it++)
            b->bPartTable->partitionBlockNames << it.key();
    });
    QObject::connect(prober, &GeomProber::gotPart, this,
                     [this, devName](QString tableBlockName, Part partInfo)
    {
        Block* b = m_blockObjects.value(devName);
        if(!b)
            return;

        addPartition(b, tableBlockName, partInfo);
    });
    QObject::connect(prober, &GeomProber::finished, this,
                     [this, devName]()
    {
        Block* b = m_blockObjects.value(devName);
        if(!b)
            return;

        b->probesDone.set(GEOM_PROBE);
        qDebug() << "Finished GEOM probe on " << b->name;
        if(b->probesDone.all())
            registerBlock(b);
    });

    QThreadPool::globalInstance()->start(prober);
}

void ObjectManager::addPartition(Block* b, const QString& tableBlockName, const Part& partInfo)
{
    auto* bp = new BlockPartition(b);

    bp->partBlockName = tableBlockName;
    bp->partitionType = partInfo.type();
    bp->number = partInfo.index();
    bp->offset = partInfo.offset();
    bp->size = partInfo.length();

    b->bPartition = bp;
}


void ObjectManager::addInterfaces(QDBusObjectPath path, QList<std::pair<QString, QDBusAbstractAdaptor*>> newInterfaces)
{
    QVariantMapMap interfaces;

    for (auto pair : newInterfaces)
    {
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
    emit InterfacesRemoved(path, ifaces);
}
