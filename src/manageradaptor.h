/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -a manager manager.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef MANAGER_H_1460792998
#define MANAGER_H_1460792998

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

typedef QMap<QString, QVariantMap> QVariantMapMap;
Q_DECLARE_METATYPE(QVariantMapMap)

typedef QMap<QDBusObjectPath, QVariantMapMap> DBUSManagerStruct;
Q_DECLARE_METATYPE(DBUSManagerStruct)

/*
 * Adaptor class for interface org.freedesktop.DBus.ObjectManager
 */
class ObjectManagerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")
    Q_CLASSINFO("D-Bus Introspection",
                ""
                "  <interface name=\"org.freedesktop.DBus.ObjectManager\">\n"
                "    <method name=\"GetManagedObjects\">\n"
                "      <arg direction=\"out\" type=\"a{oa{sa{sv}}}\" "
                "name=\"object_paths_interfaces_and_properties\">\n"
                "        <annotation value=\"DBUSManagerStruct\" "
                "name=\"com.trolltech.QtDBus.QtTypeName.Out0\"/>\n"
                "      </arg>\n"
                "    </method>\n"
                "    <signal name=\"InterfacesAdded\">\n"
                "      <arg type=\"o\" name=\"object_path\"/>\n"
                "      <arg type=\"a{sa{sv}}\" name=\"interfaces_and_properties\">\n"
                "        <annotation value=\"QVariantMapMap\" "
                "name=\"com.trolltech.QtDBus.QtTypeName.In1\"/>\n"
                "      </arg>\n"
                "    </signal>\n"
                "    <signal name=\"InterfacesRemoved\">\n"
                "      <arg type=\"o\" name=\"object_path\"/>\n"
                "      <arg type=\"as\" name=\"interfaces\"/>\n"
                "    </signal>\n"
                "  </interface>\n"
                "")
public:
    ObjectManagerAdaptor(QObject* parent);
    virtual ~ObjectManagerAdaptor() = default;

public:         // PROPERTIES
public Q_SLOTS: // METHODS
    DBUSManagerStruct GetManagedObjects();
Q_SIGNALS: // SIGNALS
    void InterfacesAdded(const QDBusObjectPath& object_path, const QVariantMapMap& interfaces_and_properties);
    void InterfacesRemoved(const QDBusObjectPath& object_path, const QStringList& interfaces);
};

#endif
