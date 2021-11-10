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

    auto addDBUSManagerStruct = [&](QObject* o, const QDBusObjectPath& dbusPath) {
        QVariantMapMap interfaces;

        for (QObject* child : o->children()) {
            if (QDBusAbstractAdaptor* adaptor = qobject_cast<QDBusAbstractAdaptor*>(child)) {
                const QString& iface =
                    adaptor->metaObject()
                        ->classInfo(adaptor->metaObject()->indexOfClassInfo("D-Bus Interface"))
                        .value();

                QVariantMap properties;
                for (int i = adaptor->metaObject()->propertyOffset();
                     i < adaptor->metaObject()->propertyCount();
                     ++i) {
                    auto propertyName = adaptor->metaObject()->property(i).name();
                    properties.insert(QString::fromLatin1(propertyName),
                                      adaptor->property(propertyName));
                }

                interfaces[iface] = properties;
            }
        }
        ret[dbusPath] = interfaces;
    };

    for (const auto& d : qAsConst(m_driveObjects))
        addDBUSManagerStruct(d.get(), d->getDbusPath());

    for (const auto& b : qAsConst(m_blockObjects))
        addDBUSManagerStruct(b.get(), b->getDbusPath());

    return ret;
}

void ObjectManager::addBlock(TBlock const& block)
{
    if (!block)
        return;

    qDebug() << "Add Block";
    m_blockObjects.insert(block->getName(), block);
    registerBlock(block);
}

void ObjectManager::updateBlock(TBlock const& block)
{
    qDebug() << "UpdateBlock";
    /*
    auto* b = m_blockObjects.value(dev);

    if (b->probesDone.all()) {
        b->probesDone.reset();
        //b->probesDone.set(FILESYSTEM_PROBE);
    }
    else
        b->needsAnotherProbe = true;
    */
}

void ObjectManager::removeBlock(TBlock const& block)
{
    qInfo() << "Unregistering Block " + block->getDbusPath().path();

    QStringList ifaces;
    ifaces << QStringLiteral("org.freedesktop.UDisks2.Block");

    if (block->getPartition() && block->getPartition()->getFilesystem())
        ifaces << QStringLiteral("org.freedesktop.UDisks2.Filesystem");
    /*
    if (block->getPartition())
        ifaces << QStringLiteral("org.freedesktop.UDisks2.Partition");
    */

    ifaces << QStringLiteral("org.freedesktop.UDisks2.Block");
    removeInterfaces(block->getDbusPath(), ifaces);
    QDBusConnection::systemBus().unregisterObject(block->getDbusPath().path());
}

void ObjectManager::addDrive(TDrive drive)
{
    if (!drive)
        return;

    m_driveObjects.insert(drive->getDeviceName(), drive);
    registerDrive(drive);
}

void ObjectManager::removeDrive(TDrive const& drive)
{
    qDebug() << "Unregistering " + drive->getDbusPath().path();
    for (const TBlock& block : drive->getBlocks())
        removeBlock(block);

    removeInterfaces(drive->getDbusPath(), {QStringLiteral("org.freedesktop.UDisks2.Drive")});
    QDBusConnection::systemBus().unregisterObject(drive->getDbusPath().path());
}

bool ObjectManager::registerBlock(const TBlock& block, bool tryPostponed)
{
    qDebug() << "RegisterBlock blockObjects :" << m_blockObjects.size();

    if (block->isUnregistered()) {
        QString devPath = block->getDbusPath().path();

        QList<std::pair<QString, QDBusAbstractAdaptor*>> interfaces;
        interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Block"),
                                     new BlockAdaptor(block.get()));

        if (block->getPartition() && block->getPartition()->getFilesystem())
            interfaces << std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Filesystem"),
                                         new FilesystemAdaptor(block.get()));

        /*
        if (block->getPartitionTable())
            interfaces <<
        std::make_pair(QStringLiteral("org.freedesktop.UDisks2.PartitionTable"),
                                         new
        PartitionTableAdaptor(block.get()));

        if (block->getPartition())
            interfaces <<
        std::make_pair(QStringLiteral("org.freedesktop.UDisks2.Partition"), new
        PartitionAdaptor(block.get()));
         */

        addInterfaces(block->getDbusPath(), interfaces);
        qInfo() << "Registering: " + devPath;
        block->setRegistered(true);
        QDBusConnection::systemBus().registerObject(devPath, block.get());
    }
    return true;
}

void ObjectManager::registerDrive(const TDrive& drive)
{
    const QString dbusPath = drive->getDbusPath().path();
    qDebug() << "Registering " + dbusPath;

    QDBusConnection::systemBus().registerObject(dbusPath, drive.get());

    addInterfaces(drive->getDbusPath(),
                  {std::make_pair("org.freedesktop.UDisks2.Drive", new DriveAdaptor(drive.get()))});
}

void ObjectManager::addInterfaces(
    const QDBusObjectPath& path,
    const QList<std::pair<QString, QDBusAbstractAdaptor*>>& newInterfaces)
{
    qDebug() << "Add Interfaces";
    QVariantMapMap interfaces;

    for (const auto& pair : newInterfaces) {
        const QString& iface = pair.first;
        qDebug() << "Add Interface: " << iface;
        QDBusAbstractAdaptor* adaptor = pair.second;

        QVariantMap properties;
        for (int i = adaptor->metaObject()->propertyOffset();
             i < adaptor->metaObject()->propertyCount();
             ++i) {
            auto propertyName = adaptor->metaObject()->property(i).name();
            properties.insert(QString::fromLatin1(propertyName), adaptor->property(propertyName));
        }

        interfaces[iface] = properties;
    }
    emit InterfacesAdded(path, interfaces);
}

void ObjectManager::removeInterfaces(const QDBusObjectPath& path, const QStringList& ifaces)
{
    qDebug() << "Remove Interfaces: " << path.path();
    emit InterfacesRemoved(path, ifaces);
}
