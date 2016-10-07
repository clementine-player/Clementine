/* This file is part of Clementine.
   Copyright 2011, John Maguire <john.maguire@gmail.com>

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

#ifndef DBUS_METATYPES_H_
#define DBUS_METATYPES_H_

#include <QMetaType>
#include <QDBusObjectPath>

Q_DECLARE_METATYPE(QList<QByteArray>)

typedef QMap<QString, QVariantMap> InterfacesAndProperties;
typedef QMap<QDBusObjectPath, InterfacesAndProperties> ManagedObjectList;

Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(ManagedObjectList)

#endif  // DBUS_METATYPES_H_
