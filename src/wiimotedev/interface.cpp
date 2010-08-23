/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wiimotedev/interface.h"

QDBusArgument& operator<<(QDBusArgument& argument, const irpoint& point) {
    argument.beginStructure();
    argument << point.size << point.x << point.y;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, irpoint& point) {
    argument.beginStructure();
    argument >> point.size >> point.x >> point.y;
    argument.endStructure();
    return argument;
}

QDBusArgument& operator<<(QDBusArgument& argument, const accdata& acc) {
    argument.beginStructure();
    argument << acc.x << acc.y << acc.z << acc.pitch << acc.roll;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, accdata& acc) {
    argument.beginStructure();
    argument >> acc.x >> acc.y >> acc.z >> acc.pitch >> acc.roll;
    argument.endStructure();
    return argument;
}

QDBusArgument& operator<<(QDBusArgument& argument, const stickdata& stick) {
    argument.beginStructure();
    argument << stick.x << stick.y;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, stickdata& stick) {
    argument.beginStructure();
    argument >> stick.x >> stick.y;
    argument.endStructure();
    return argument;
}


DBusDeviceEventsInterface::DBusDeviceEventsInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
:QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent) { 
  QWIIMOTEDEV_REGISTER_META_TYPES 
}

QDBusReply < bool> DBusDeviceEventsInterface::dbusWiimoteGetRumbleStatus(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteGetRumbleStatus", argumentList);
}

QDBusReply < bool> DBusDeviceEventsInterface::dbusWiimoteSetLedStatus(quint32 id, quint32 status) {
    QList<QVariant> argumentList;
    argumentList << id;
    argumentList << status;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteSetLedStatus", argumentList);
}

QDBusReply < bool> DBusDeviceEventsInterface::dbusWiimoteSetRumbleStatus(quint32 id, bool status) {
    QList<QVariant> argumentList;
    argumentList << id << status;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteSetRumbleStatus", argumentList);
}

QDBusReply < QList <uint> > DBusDeviceEventsInterface::dbusGetWiimoteList() {
    return call(QDBus::Block, "dbusGetWiimoteList");
}

QDBusReply < QStringList> DBusDeviceEventsInterface::dbusGetUnregistredWiimoteList() {
    return call(QDBus::Block, "dbusGetUnregistredWiimoteList");
}

QDBusReply < bool> DBusDeviceEventsInterface::dbusIsClassicConnected(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusIsClassicConnected", argumentList);
}

QDBusReply < bool> DBusDeviceEventsInterface::dbusIsNunchukConnected(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusIsNunchukConnected", argumentList);
}

QDBusReply < bool> DBusDeviceEventsInterface::dbusIsWiimoteConnected(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusIsWiimoteConnected", argumentList);
}

QDBusReply < quint8> DBusDeviceEventsInterface::dbusWiimoteAverageLatency(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteAverageLatency", argumentList);
}

QDBusReply < quint8> DBusDeviceEventsInterface::dbusWiimoteCurrentLatency(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteCurrentLatency", argumentList);
}

QDBusReply < quint8> DBusDeviceEventsInterface::dbusWiimoteGetStatus(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteGetStatus", argumentList);
}


QDBusReply < quint8> DBusDeviceEventsInterface::dbusWiimoteGetLedStatus(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteGetLedStatus", argumentList);
}

QDBusReply < quint32> DBusDeviceEventsInterface::dbusWiimoteGetBatteryLife(quint32 id) {
    QList<QVariant> argumentList;
    argumentList << id;
    return callWithArgumentList(QDBus::Block, "dbusWiimoteGetBatteryLife", argumentList);
}
