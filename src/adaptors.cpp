/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -a gen.h:gen.cpp org.freedesktop.UDisks2.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "adaptors.h"
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMetaObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

QDBusArgument& operator<<(QDBusArgument& argument, const Configuration& cfg)
{
    argument.beginStructure();
    argument << cfg.type << cfg.data;
    argument.endStructure();
    return argument;
}
const QDBusArgument& operator>>(const QDBusArgument& argument, Configuration& cfg)
{
    argument.beginStructure();
    argument >> cfg.type >> cfg.data;
    argument.endStructure();
    return argument;
}

/*
 * Implementation of adaptor class BlockAdaptor
 */

BlockAdaptor::BlockAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

BlockAdaptor::~BlockAdaptor()
{
    // destructor
}

ConfigurationList BlockAdaptor::configuration() const
{
    // get the value of property Configuration
    return qvariant_cast<ConfigurationList>(parent()->property("Configuration"));
}

QDBusObjectPath BlockAdaptor::cryptoBackingDevice() const
{
    // get the value of property CryptoBackingDevice
    return qvariant_cast<QDBusObjectPath>(parent()->property("CryptoBackingDevice"));
}

QByteArray BlockAdaptor::device() const
{
    // get the value of property Device
    return qvariant_cast<QByteArray>(parent()->property("Device"));
}

qulonglong BlockAdaptor::deviceNumber() const
{
    // get the value of property DeviceNumber
    return qvariant_cast<qulonglong>(parent()->property("DeviceNumber"));
}

QDBusObjectPath BlockAdaptor::drive() const
{
    // get the value of property Drive
    return qvariant_cast<QDBusObjectPath>(parent()->property("Drive"));
}

bool BlockAdaptor::hintAuto() const
{
    // get the value of property HintAuto
    return qvariant_cast<bool>(parent()->property("HintAuto"));
}

QString BlockAdaptor::hintIconName() const
{
    // get the value of property HintIconName
    return qvariant_cast<QString>(parent()->property("HintIconName"));
}

bool BlockAdaptor::hintIgnore() const
{
    // get the value of property HintIgnore
    return qvariant_cast<bool>(parent()->property("HintIgnore"));
}

QString BlockAdaptor::hintName() const
{
    // get the value of property HintName
    return qvariant_cast<QString>(parent()->property("HintName"));
}

bool BlockAdaptor::hintPartitionable() const
{
    // get the value of property HintPartitionable
    return qvariant_cast<bool>(parent()->property("HintPartitionable"));
}

QString BlockAdaptor::hintSymbolicIconName() const
{
    // get the value of property HintSymbolicIconName
    return qvariant_cast<QString>(parent()->property("HintSymbolicIconName"));
}

bool BlockAdaptor::hintSystem() const
{
    // get the value of property HintSystem
    return qvariant_cast<bool>(parent()->property("HintSystem"));
}

QString BlockAdaptor::id() const
{
    // get the value of property Id
    return qvariant_cast<QString>(parent()->property("Id"));
}

QString BlockAdaptor::idLabel() const
{
    // get the value of property IdLabel
    return qvariant_cast<QString>(parent()->property("IdLabel"));
}

QString BlockAdaptor::idType() const
{
    // get the value of property IdType
    return qvariant_cast<QString>(parent()->property("IdType"));
}

QString BlockAdaptor::idUUID() const
{
    // get the value of property IdUUID
    return qvariant_cast<QString>(parent()->property("IdUUID"));
}

QString BlockAdaptor::idUsage() const
{
    // get the value of property IdUsage
    return qvariant_cast<QString>(parent()->property("IdUsage"));
}

QString BlockAdaptor::idVersion() const
{
    // get the value of property IdVersion
    return qvariant_cast<QString>(parent()->property("IdVersion"));
}

QDBusObjectPath BlockAdaptor::mDRaid() const
{
    // get the value of property MDRaid
    return qvariant_cast<QDBusObjectPath>(parent()->property("MDRaid"));
}

QDBusObjectPath BlockAdaptor::mDRaidMember() const
{
    // get the value of property MDRaidMember
    return qvariant_cast<QDBusObjectPath>(parent()->property("MDRaidMember"));
}

QByteArray BlockAdaptor::preferredDevice() const
{
    // get the value of property PreferredDevice
    return qvariant_cast<QByteArray>(parent()->property("PreferredDevice"));
}

bool BlockAdaptor::readOnly() const
{
    // get the value of property ReadOnly
    return qvariant_cast<bool>(parent()->property("ReadOnly"));
}

qulonglong BlockAdaptor::size() const
{
    // get the value of property Size
    return qvariant_cast<qulonglong>(parent()->property("Size"));
}

QByteArrayList BlockAdaptor::symlinks() const
{
    // get the value of property Symlinks
    return qvariant_cast<QByteArrayList>(parent()->property("Symlinks"));
}

void BlockAdaptor::AddConfigurationItem(Configuration item, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.AddConfigurationItem
    QMetaObject::invokeMethod(parent(), "AddConfigurationItem", Q_ARG(Configuration, item), Q_ARG(QVariantMap, options));
}

void BlockAdaptor::Format(const QString& type, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.Format
    QMetaObject::invokeMethod(parent(), "Format", Q_ARG(QString, type), Q_ARG(QVariantMap, options));
}

ConfigurationList BlockAdaptor::GetSecretConfiguration(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.GetSecretConfiguration
    ConfigurationList configuration;
    QMetaObject::invokeMethod(parent(), "GetSecretConfiguration", Q_RETURN_ARG(ConfigurationList, configuration), Q_ARG(QVariantMap, options));
    return configuration;
}

QDBusUnixFileDescriptor BlockAdaptor::OpenForBackup(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.OpenForBackup
    QDBusUnixFileDescriptor fd;
    QMetaObject::invokeMethod(parent(), "OpenForBackup", Q_RETURN_ARG(QDBusUnixFileDescriptor, fd), Q_ARG(QVariantMap, options));
    return fd;
}

QDBusUnixFileDescriptor BlockAdaptor::OpenForBenchmark(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.OpenForBenchmark
    QDBusUnixFileDescriptor fd;
    QMetaObject::invokeMethod(parent(), "OpenForBenchmark", Q_RETURN_ARG(QDBusUnixFileDescriptor, fd), Q_ARG(QVariantMap, options));
    return fd;
}

QDBusUnixFileDescriptor BlockAdaptor::OpenForRestore(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.OpenForRestore
    QDBusUnixFileDescriptor fd;
    QMetaObject::invokeMethod(parent(), "OpenForRestore", Q_RETURN_ARG(QDBusUnixFileDescriptor, fd), Q_ARG(QVariantMap, options));
    return fd;
}

void BlockAdaptor::RemoveConfigurationItem(Configuration item, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.RemoveConfigurationItem
    QMetaObject::invokeMethod(parent(), "RemoveConfigurationItem", Q_ARG(Configuration, item), Q_ARG(QVariantMap, options));
}

void BlockAdaptor::Rescan(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.Rescan
    QMetaObject::invokeMethod(parent(), "Rescan", Q_ARG(QVariantMap, options));
}

void BlockAdaptor::UpdateConfigurationItem(Configuration old_item, Configuration new_item, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Block.UpdateConfigurationItem
    QMetaObject::invokeMethod(parent(), "UpdateConfigurationItem", Q_ARG(Configuration, old_item), Q_ARG(Configuration, new_item), Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class DriveAdaptor
 */

DriveAdaptor::DriveAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DriveAdaptor::~DriveAdaptor()
{
    // destructor
}

bool DriveAdaptor::canPowerOff() const
{
    // get the value of property CanPowerOff
    return qvariant_cast<bool>(parent()->property("CanPowerOff"));
}

QVariantMap DriveAdaptor::configuration() const
{
    // get the value of property Configuration
    return qvariant_cast<QVariantMap>(parent()->property("Configuration"));
}

QString DriveAdaptor::connectionBus() const
{
    // get the value of property ConnectionBus
    return qvariant_cast<QString>(parent()->property("ConnectionBus"));
}

bool DriveAdaptor::ejectable() const
{
    // get the value of property Ejectable
    return qvariant_cast<bool>(parent()->property("Ejectable"));
}

QString DriveAdaptor::id() const
{
    // get the value of property Id
    return qvariant_cast<QString>(parent()->property("Id"));
}

QString DriveAdaptor::media() const
{
    // get the value of property Media
    return qvariant_cast<QString>(parent()->property("Media"));
}

bool DriveAdaptor::mediaAvailable() const
{
    // get the value of property MediaAvailable
    return qvariant_cast<bool>(parent()->property("MediaAvailable"));
}

bool DriveAdaptor::mediaChangeDetected() const
{
    // get the value of property MediaChangeDetected
    return qvariant_cast<bool>(parent()->property("MediaChangeDetected"));
}

QStringList DriveAdaptor::mediaCompatibility() const
{
    // get the value of property MediaCompatibility
    return qvariant_cast<QStringList>(parent()->property("MediaCompatibility"));
}

bool DriveAdaptor::mediaRemovable() const
{
    // get the value of property MediaRemovable
    return qvariant_cast<bool>(parent()->property("MediaRemovable"));
}

QString DriveAdaptor::model() const
{
    // get the value of property Model
    return qvariant_cast<QString>(parent()->property("Model"));
}

bool DriveAdaptor::optical() const
{
    // get the value of property Optical
    return qvariant_cast<bool>(parent()->property("Optical"));
}

bool DriveAdaptor::opticalBlank() const
{
    // get the value of property OpticalBlank
    return qvariant_cast<bool>(parent()->property("OpticalBlank"));
}

uint DriveAdaptor::opticalNumAudioTracks() const
{
    // get the value of property OpticalNumAudioTracks
    return qvariant_cast<uint>(parent()->property("OpticalNumAudioTracks"));
}

uint DriveAdaptor::opticalNumDataTracks() const
{
    // get the value of property OpticalNumDataTracks
    return qvariant_cast<uint>(parent()->property("OpticalNumDataTracks"));
}

uint DriveAdaptor::opticalNumSessions() const
{
    // get the value of property OpticalNumSessions
    return qvariant_cast<uint>(parent()->property("OpticalNumSessions"));
}

uint DriveAdaptor::opticalNumTracks() const
{
    // get the value of property OpticalNumTracks
    return qvariant_cast<uint>(parent()->property("OpticalNumTracks"));
}

bool DriveAdaptor::removable() const
{
    // get the value of property Removable
    return qvariant_cast<bool>(parent()->property("Removable"));
}

QString DriveAdaptor::revision() const
{
    // get the value of property Revision
    return qvariant_cast<QString>(parent()->property("Revision"));
}

int DriveAdaptor::rotationRate() const
{
    // get the value of property RotationRate
    return qvariant_cast<int>(parent()->property("RotationRate"));
}

QString DriveAdaptor::seat() const
{
    // get the value of property Seat
    return qvariant_cast<QString>(parent()->property("Seat"));
}

QString DriveAdaptor::serial() const
{
    // get the value of property Serial
    return qvariant_cast<QString>(parent()->property("Serial"));
}

QString DriveAdaptor::siblingId() const
{
    // get the value of property SiblingId
    return qvariant_cast<QString>(parent()->property("SiblingId"));
}

qulonglong DriveAdaptor::size() const
{
    // get the value of property Size
    return qvariant_cast<qulonglong>(parent()->property("Size"));
}

QString DriveAdaptor::sortKey() const
{
    // get the value of property SortKey
    return qvariant_cast<QString>(parent()->property("SortKey"));
}

qulonglong DriveAdaptor::timeDetected() const
{
    // get the value of property TimeDetected
    return qvariant_cast<qulonglong>(parent()->property("TimeDetected"));
}

qulonglong DriveAdaptor::timeMediaDetected() const
{
    // get the value of property TimeMediaDetected
    return qvariant_cast<qulonglong>(parent()->property("TimeMediaDetected"));
}

QString DriveAdaptor::vendor() const
{
    // get the value of property Vendor
    return qvariant_cast<QString>(parent()->property("Vendor"));
}

QString DriveAdaptor::wWN() const
{
    // get the value of property WWN
    return qvariant_cast<QString>(parent()->property("WWN"));
}

void DriveAdaptor::Eject(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Eject
    QMetaObject::invokeMethod(parent(), "Eject", Q_ARG(QVariantMap, options));
}

void DriveAdaptor::PowerOff(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.PowerOff
    QMetaObject::invokeMethod(parent(), "PowerOff", Q_ARG(QVariantMap, options));
}

void DriveAdaptor::SetConfiguration(const QVariantMap& value, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.SetConfiguration
    QMetaObject::invokeMethod(parent(), "SetConfiguration", Q_ARG(QVariantMap, value), Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class AtaAdaptor
 */

AtaAdaptor::AtaAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

AtaAdaptor::~AtaAdaptor()
{
    // destructor
}

bool AtaAdaptor::aamEnabled() const
{
    // get the value of property AamEnabled
    return qvariant_cast<bool>(parent()->property("AamEnabled"));
}

bool AtaAdaptor::aamSupported() const
{
    // get the value of property AamSupported
    return qvariant_cast<bool>(parent()->property("AamSupported"));
}

int AtaAdaptor::aamVendorRecommendedValue() const
{
    // get the value of property AamVendorRecommendedValue
    return qvariant_cast<int>(parent()->property("AamVendorRecommendedValue"));
}

bool AtaAdaptor::apmEnabled() const
{
    // get the value of property ApmEnabled
    return qvariant_cast<bool>(parent()->property("ApmEnabled"));
}

bool AtaAdaptor::apmSupported() const
{
    // get the value of property ApmSupported
    return qvariant_cast<bool>(parent()->property("ApmSupported"));
}

bool AtaAdaptor::pmEnabled() const
{
    // get the value of property PmEnabled
    return qvariant_cast<bool>(parent()->property("PmEnabled"));
}

bool AtaAdaptor::pmSupported() const
{
    // get the value of property PmSupported
    return qvariant_cast<bool>(parent()->property("PmSupported"));
}

bool AtaAdaptor::readLookaheadEnabled() const
{
    // get the value of property ReadLookaheadEnabled
    return qvariant_cast<bool>(parent()->property("ReadLookaheadEnabled"));
}

bool AtaAdaptor::readLookaheadSupported() const
{
    // get the value of property ReadLookaheadSupported
    return qvariant_cast<bool>(parent()->property("ReadLookaheadSupported"));
}

int AtaAdaptor::securityEnhancedEraseUnitMinutes() const
{
    // get the value of property SecurityEnhancedEraseUnitMinutes
    return qvariant_cast<int>(parent()->property("SecurityEnhancedEraseUnitMinutes"));
}

int AtaAdaptor::securityEraseUnitMinutes() const
{
    // get the value of property SecurityEraseUnitMinutes
    return qvariant_cast<int>(parent()->property("SecurityEraseUnitMinutes"));
}

bool AtaAdaptor::securityFrozen() const
{
    // get the value of property SecurityFrozen
    return qvariant_cast<bool>(parent()->property("SecurityFrozen"));
}

bool AtaAdaptor::smartEnabled() const
{
    // get the value of property SmartEnabled
    return qvariant_cast<bool>(parent()->property("SmartEnabled"));
}

bool AtaAdaptor::smartFailing() const
{
    // get the value of property SmartFailing
    return qvariant_cast<bool>(parent()->property("SmartFailing"));
}

int AtaAdaptor::smartNumAttributesFailedInThePast() const
{
    // get the value of property SmartNumAttributesFailedInThePast
    return qvariant_cast<int>(parent()->property("SmartNumAttributesFailedInThePast"));
}

int AtaAdaptor::smartNumAttributesFailing() const
{
    // get the value of property SmartNumAttributesFailing
    return qvariant_cast<int>(parent()->property("SmartNumAttributesFailing"));
}

qlonglong AtaAdaptor::smartNumBadSectors() const
{
    // get the value of property SmartNumBadSectors
    return qvariant_cast<qlonglong>(parent()->property("SmartNumBadSectors"));
}

qulonglong AtaAdaptor::smartPowerOnSeconds() const
{
    // get the value of property SmartPowerOnSeconds
    return qvariant_cast<qulonglong>(parent()->property("SmartPowerOnSeconds"));
}

int AtaAdaptor::smartSelftestPercentRemaining() const
{
    // get the value of property SmartSelftestPercentRemaining
    return qvariant_cast<int>(parent()->property("SmartSelftestPercentRemaining"));
}

QString AtaAdaptor::smartSelftestStatus() const
{
    // get the value of property SmartSelftestStatus
    return qvariant_cast<QString>(parent()->property("SmartSelftestStatus"));
}

bool AtaAdaptor::smartSupported() const
{
    // get the value of property SmartSupported
    return qvariant_cast<bool>(parent()->property("SmartSupported"));
}

double AtaAdaptor::smartTemperature() const
{
    // get the value of property SmartTemperature
    return qvariant_cast<double>(parent()->property("SmartTemperature"));
}

qulonglong AtaAdaptor::smartUpdated() const
{
    // get the value of property SmartUpdated
    return qvariant_cast<qulonglong>(parent()->property("SmartUpdated"));
}

bool AtaAdaptor::writeCacheEnabled() const
{
    // get the value of property WriteCacheEnabled
    return qvariant_cast<bool>(parent()->property("WriteCacheEnabled"));
}

bool AtaAdaptor::writeCacheSupported() const
{
    // get the value of property WriteCacheSupported
    return qvariant_cast<bool>(parent()->property("WriteCacheSupported"));
}

uchar AtaAdaptor::PmGetState(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.PmGetState
    uchar state;
    QMetaObject::invokeMethod(parent(), "PmGetState", Q_RETURN_ARG(uchar, state), Q_ARG(QVariantMap, options));
    return state;
}

void AtaAdaptor::PmStandby(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.PmStandby
    QMetaObject::invokeMethod(parent(), "PmStandby", Q_ARG(QVariantMap, options));
}

void AtaAdaptor::PmWakeup(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.PmWakeup
    QMetaObject::invokeMethod(parent(), "PmWakeup", Q_ARG(QVariantMap, options));
}

void AtaAdaptor::SecurityEraseUnit(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.SecurityEraseUnit
    QMetaObject::invokeMethod(parent(), "SecurityEraseUnit", Q_ARG(QVariantMap, options));
}

QVariantList AtaAdaptor::SmartGetAttributes(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.SmartGetAttributes
    QVariantList attributes;
    QMetaObject::invokeMethod(parent(), "SmartGetAttributes", Q_RETURN_ARG(QVariantList, attributes), Q_ARG(QVariantMap, options));
    return attributes;
}

void AtaAdaptor::SmartSelftestAbort(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.SmartSelftestAbort
    QMetaObject::invokeMethod(parent(), "SmartSelftestAbort", Q_ARG(QVariantMap, options));
}

void AtaAdaptor::SmartSelftestStart(const QString& type, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.SmartSelftestStart
    QMetaObject::invokeMethod(parent(), "SmartSelftestStart", Q_ARG(QString, type), Q_ARG(QVariantMap, options));
}

void AtaAdaptor::SmartSetEnabled(bool value, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.SmartSetEnabled
    QMetaObject::invokeMethod(parent(), "SmartSetEnabled", Q_ARG(bool, value), Q_ARG(QVariantMap, options));
}

void AtaAdaptor::SmartUpdate(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Drive.Ata.SmartUpdate
    QMetaObject::invokeMethod(parent(), "SmartUpdate", Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class EncryptedAdaptor
 */

EncryptedAdaptor::EncryptedAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

EncryptedAdaptor::~EncryptedAdaptor()
{
    // destructor
}

void EncryptedAdaptor::ChangePassphrase(const QString& passphrase, const QString& new_passphrase, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Encrypted.ChangePassphrase
    QMetaObject::invokeMethod(parent(), "ChangePassphrase", Q_ARG(QString, passphrase), Q_ARG(QString, new_passphrase), Q_ARG(QVariantMap, options));
}

void EncryptedAdaptor::Lock(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Encrypted.Lock
    QMetaObject::invokeMethod(parent(), "Lock", Q_ARG(QVariantMap, options));
}

QDBusObjectPath EncryptedAdaptor::Unlock(const QString& passphrase, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Encrypted.Unlock
    QDBusObjectPath cleartext_device;
    QMetaObject::invokeMethod(parent(), "Unlock", Q_RETURN_ARG(QDBusObjectPath, cleartext_device), Q_ARG(QString, passphrase), Q_ARG(QVariantMap, options));
    return cleartext_device;
}

/*
 * Implementation of adaptor class FilesystemAdaptor
 */

FilesystemAdaptor::FilesystemAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

FilesystemAdaptor::~FilesystemAdaptor()
{
    // destructor
}

QByteArrayList FilesystemAdaptor::mountPoints() const
{
    // get the value of property MountPoints
    return qvariant_cast<QByteArrayList>(parent()->property("MountPoints"));
}

QString FilesystemAdaptor::Mount(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Filesystem.Mount
    QString mount_path;
    QMetaObject::invokeMethod(parent(), "Mount", Q_RETURN_ARG(QString, mount_path), Q_ARG(QVariantMap, options));
    return mount_path;
}

void FilesystemAdaptor::SetLabel(const QString& label, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Filesystem.SetLabel
    QMetaObject::invokeMethod(parent(), "SetLabel", Q_ARG(QString, label), Q_ARG(QVariantMap, options));
}

void FilesystemAdaptor::Unmount(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Filesystem.Unmount
    QMetaObject::invokeMethod(parent(), "Unmount", Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class JobAdaptor
 */

JobAdaptor::JobAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

JobAdaptor::~JobAdaptor()
{
    // destructor
}

qulonglong JobAdaptor::bytes() const
{
    // get the value of property Bytes
    return qvariant_cast<qulonglong>(parent()->property("Bytes"));
}

bool JobAdaptor::cancelable() const
{
    // get the value of property Cancelable
    return qvariant_cast<bool>(parent()->property("Cancelable"));
}

qulonglong JobAdaptor::expectedEndTime() const
{
    // get the value of property ExpectedEndTime
    return qvariant_cast<qulonglong>(parent()->property("ExpectedEndTime"));
}

QList<QDBusObjectPath> JobAdaptor::objects() const
{
    // get the value of property Objects
    return qvariant_cast<QList<QDBusObjectPath> >(parent()->property("Objects"));
}

QString JobAdaptor::operation() const
{
    // get the value of property Operation
    return qvariant_cast<QString>(parent()->property("Operation"));
}

double JobAdaptor::progress() const
{
    // get the value of property Progress
    return qvariant_cast<double>(parent()->property("Progress"));
}

bool JobAdaptor::progressValid() const
{
    // get the value of property ProgressValid
    return qvariant_cast<bool>(parent()->property("ProgressValid"));
}

qulonglong JobAdaptor::rate() const
{
    // get the value of property Rate
    return qvariant_cast<qulonglong>(parent()->property("Rate"));
}

qulonglong JobAdaptor::startTime() const
{
    // get the value of property StartTime
    return qvariant_cast<qulonglong>(parent()->property("StartTime"));
}

uint JobAdaptor::startedByUID() const
{
    // get the value of property StartedByUID
    return qvariant_cast<uint>(parent()->property("StartedByUID"));
}

void JobAdaptor::Cancel(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Job.Cancel
    QMetaObject::invokeMethod(parent(), "Cancel", Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class LoopAdaptor
 */

LoopAdaptor::LoopAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

LoopAdaptor::~LoopAdaptor()
{
    // destructor
}

bool LoopAdaptor::autoclear() const
{
    // get the value of property Autoclear
    return qvariant_cast<bool>(parent()->property("Autoclear"));
}

QByteArray LoopAdaptor::backingFile() const
{
    // get the value of property BackingFile
    return qvariant_cast<QByteArray>(parent()->property("BackingFile"));
}

uint LoopAdaptor::setupByUID() const
{
    // get the value of property SetupByUID
    return qvariant_cast<uint>(parent()->property("SetupByUID"));
}

void LoopAdaptor::Delete(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Loop.Delete
    QMetaObject::invokeMethod(parent(), "Delete", Q_ARG(QVariantMap, options));
}

void LoopAdaptor::SetAutoclear(bool value, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Loop.SetAutoclear
    QMetaObject::invokeMethod(parent(), "SetAutoclear", Q_ARG(bool, value), Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class MDRaidAdaptor
 */

MDRaidAdaptor::MDRaidAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

MDRaidAdaptor::~MDRaidAdaptor()
{
    // destructor
}

QVariantList MDRaidAdaptor::activeDevices() const
{
    // get the value of property ActiveDevices
    return qvariant_cast<QVariantList>(parent()->property("ActiveDevices"));
}

QByteArray MDRaidAdaptor::bitmapLocation() const
{
    // get the value of property BitmapLocation
    return qvariant_cast<QByteArray>(parent()->property("BitmapLocation"));
}

qulonglong MDRaidAdaptor::chunkSize() const
{
    // get the value of property ChunkSize
    return qvariant_cast<qulonglong>(parent()->property("ChunkSize"));
}

uint MDRaidAdaptor::degraded() const
{
    // get the value of property Degraded
    return qvariant_cast<uint>(parent()->property("Degraded"));
}

QString MDRaidAdaptor::level() const
{
    // get the value of property Level
    return qvariant_cast<QString>(parent()->property("Level"));
}

QString MDRaidAdaptor::name() const
{
    // get the value of property Name
    return qvariant_cast<QString>(parent()->property("Name"));
}

uint MDRaidAdaptor::numDevices() const
{
    // get the value of property NumDevices
    return qvariant_cast<uint>(parent()->property("NumDevices"));
}

qulonglong MDRaidAdaptor::size() const
{
    // get the value of property Size
    return qvariant_cast<qulonglong>(parent()->property("Size"));
}

QString MDRaidAdaptor::syncAction() const
{
    // get the value of property SyncAction
    return qvariant_cast<QString>(parent()->property("SyncAction"));
}

double MDRaidAdaptor::syncCompleted() const
{
    // get the value of property SyncCompleted
    return qvariant_cast<double>(parent()->property("SyncCompleted"));
}

qulonglong MDRaidAdaptor::syncRate() const
{
    // get the value of property SyncRate
    return qvariant_cast<qulonglong>(parent()->property("SyncRate"));
}

qulonglong MDRaidAdaptor::syncRemainingTime() const
{
    // get the value of property SyncRemainingTime
    return qvariant_cast<qulonglong>(parent()->property("SyncRemainingTime"));
}

QString MDRaidAdaptor::uUID() const
{
    // get the value of property UUID
    return qvariant_cast<QString>(parent()->property("UUID"));
}

void MDRaidAdaptor::AddDevice(const QDBusObjectPath& device, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.MDRaid.AddDevice
    QMetaObject::invokeMethod(parent(), "AddDevice", Q_ARG(QDBusObjectPath, device), Q_ARG(QVariantMap, options));
}

void MDRaidAdaptor::RemoveDevice(const QDBusObjectPath& device, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.MDRaid.RemoveDevice
    QMetaObject::invokeMethod(parent(), "RemoveDevice", Q_ARG(QDBusObjectPath, device), Q_ARG(QVariantMap, options));
}

void MDRaidAdaptor::RequestSyncAction(const QString& sync_action, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.MDRaid.RequestSyncAction
    QMetaObject::invokeMethod(parent(), "RequestSyncAction", Q_ARG(QString, sync_action), Q_ARG(QVariantMap, options));
}

void MDRaidAdaptor::SetBitmapLocation(const QByteArray& value, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.MDRaid.SetBitmapLocation
    QMetaObject::invokeMethod(parent(), "SetBitmapLocation", Q_ARG(QByteArray, value), Q_ARG(QVariantMap, options));
}

void MDRaidAdaptor::Start(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.MDRaid.Start
    QMetaObject::invokeMethod(parent(), "Start", Q_ARG(QVariantMap, options));
}

void MDRaidAdaptor::Stop(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.MDRaid.Stop
    QMetaObject::invokeMethod(parent(), "Stop", Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class ManagerAdaptor
 */

ManagerAdaptor::ManagerAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ManagerAdaptor::~ManagerAdaptor()
{
    // destructor
}

QString ManagerAdaptor::version() const
{
    // get the value of property Version
    return qvariant_cast<QString>(parent()->property("Version"));
}

QDBusObjectPath ManagerAdaptor::LoopSetup(const QDBusUnixFileDescriptor& fd, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Manager.LoopSetup
    QDBusObjectPath resulting_device;
    QMetaObject::invokeMethod(parent(), "LoopSetup", Q_RETURN_ARG(QDBusObjectPath, resulting_device), Q_ARG(QDBusUnixFileDescriptor, fd), Q_ARG(QVariantMap, options));
    return resulting_device;
}

QDBusObjectPath ManagerAdaptor::MDRaidCreate(const QList<QDBusObjectPath>& blocks, const QString& level, const QString& name, qulonglong chunk, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Manager.MDRaidCreate
    QDBusObjectPath resulting_array;
    QMetaObject::invokeMethod(parent(), "MDRaidCreate", Q_RETURN_ARG(QDBusObjectPath, resulting_array), Q_ARG(QList<QDBusObjectPath>, blocks), Q_ARG(QString, level), Q_ARG(QString, name), Q_ARG(qulonglong, chunk), Q_ARG(QVariantMap, options));
    return resulting_array;
}

/*
 * Implementation of adaptor class PartitionAdaptor
 */

PartitionAdaptor::PartitionAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

PartitionAdaptor::~PartitionAdaptor()
{
    // destructor
}

qulonglong PartitionAdaptor::flags() const
{
    // get the value of property Flags
    return qvariant_cast<qulonglong>(parent()->property("Flags"));
}

bool PartitionAdaptor::isContained() const
{
    // get the value of property IsContained
    return qvariant_cast<bool>(parent()->property("IsContained"));
}

bool PartitionAdaptor::isContainer() const
{
    // get the value of property IsContainer
    return qvariant_cast<bool>(parent()->property("IsContainer"));
}

QString PartitionAdaptor::name() const
{
    // get the value of property Name
    return qvariant_cast<QString>(parent()->property("Partition_Name"));
}

uint PartitionAdaptor::number() const
{
    // get the value of property Number
    return qvariant_cast<uint>(parent()->property("Number"));
}

qulonglong PartitionAdaptor::offset() const
{
    // get the value of property Offset
    return qvariant_cast<qulonglong>(parent()->property("Offset"));
}

qulonglong PartitionAdaptor::size() const
{
    // get the value of property Size
    return qvariant_cast<qulonglong>(parent()->property("Partition_Size"));
}

QDBusObjectPath PartitionAdaptor::table() const
{
    // get the value of property Table
    return qvariant_cast<QDBusObjectPath>(parent()->property("Table"));
}

QString PartitionAdaptor::type() const
{
    // get the value of property Type
    return qvariant_cast<QString>(parent()->property("Partition_Type"));
}

QString PartitionAdaptor::uUID() const
{
    // get the value of property UUID
    return qvariant_cast<QString>(parent()->property("UUID"));
}

void PartitionAdaptor::Delete(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Partition.Delete
    QMetaObject::invokeMethod(parent(), "Delete", Q_ARG(QVariantMap, options));
}

void PartitionAdaptor::SetFlags(qulonglong flags, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Partition.SetFlags
    QMetaObject::invokeMethod(parent(), "SetFlags", Q_ARG(qulonglong, flags), Q_ARG(QVariantMap, options));
}

void PartitionAdaptor::SetName(const QString& name, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Partition.SetName
    QMetaObject::invokeMethod(parent(), "SetName", Q_ARG(QString, name), Q_ARG(QVariantMap, options));
}

void PartitionAdaptor::SetType(const QString& type, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Partition.SetType
    QMetaObject::invokeMethod(parent(), "SetType", Q_ARG(QString, type), Q_ARG(QVariantMap, options));
}

/*
 * Implementation of adaptor class PartitionTableAdaptor
 */

PartitionTableAdaptor::PartitionTableAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

PartitionTableAdaptor::~PartitionTableAdaptor()
{
    // destructor
}

QString PartitionTableAdaptor::type() const
{
    // get the value of property Type
    return qvariant_cast<QString>(parent()->property("PartitionTable_Type"));
}

QDBusObjectPath PartitionTableAdaptor::CreatePartition(qulonglong offset, qulonglong size, const QString& type, const QString& name, const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.PartitionTable.CreatePartition
    QDBusObjectPath created_partition;
    QMetaObject::invokeMethod(parent(), "CreatePartition", Q_RETURN_ARG(QDBusObjectPath, created_partition), Q_ARG(qulonglong, offset), Q_ARG(qulonglong, size), Q_ARG(QString, type), Q_ARG(QString, name), Q_ARG(QVariantMap, options));
    return created_partition;
}

/*
 * Implementation of adaptor class SwapspaceAdaptor
 */

SwapspaceAdaptor::SwapspaceAdaptor(QObject* parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

SwapspaceAdaptor::~SwapspaceAdaptor()
{
    // destructor
}

bool SwapspaceAdaptor::active() const
{
    // get the value of property Active
    return qvariant_cast<bool>(parent()->property("Active"));
}

void SwapspaceAdaptor::Start(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Swapspace.Start
    QMetaObject::invokeMethod(parent(), "Start", Q_ARG(QVariantMap, options));
}

void SwapspaceAdaptor::Stop(const QVariantMap& options)
{
    // handle method call org.freedesktop.UDisks2.Swapspace.Stop
    QMetaObject::invokeMethod(parent(), "Stop", Q_ARG(QVariantMap, options));
}
