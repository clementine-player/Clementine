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

#ifndef FILESYSTEMDEVICEENGINE_H
#define FILESYSTEMDEVICEENGINE_H

#include "deviceengine.h"

#include <QMutex>
#include <QStringList>

#include <boost/scoped_ptr.hpp>

class OrgFreedesktopUDisksInterface;

class QDBusObjectPath;

class FilesystemDeviceEngine : public DeviceEngine {
  Q_OBJECT

public:
  FilesystemDeviceEngine();
  ~FilesystemDeviceEngine();

  enum Field {
    Field_MountPath = LastDeviceEngineField,
    Field_DbusPath,

    LastFilesystemDeviceEngineField
  };

  QStringList DeviceUniqueIDs();
  QVariant DeviceInfo(const QString& id, int field);

protected:
  void Init();

private slots:
  void DBusDeviceAdded(const QDBusObjectPath& path);
  void DBusDeviceRemoved(const QDBusObjectPath& path);
  void DBusDeviceChanged(const QDBusObjectPath& path);

private:
  struct DeviceData {
    DeviceData() : suitable(false), device_size(0) {}

    QString unique_id() const;

    bool suitable;
    QString dbus_path;
    QString drive_serial;
    QString drive_model;
    QString drive_vendor;
    QString device_presentation_name;
    QString device_presentation_icon_name;
    QStringList device_mount_paths;
    quint64 device_size;
  };

  DeviceData ReadDeviceData(const QDBusObjectPath& path) const;

  // You MUST hold the mutex while calling this function
  QString FindUniqueIdByPath(const QDBusObjectPath& path) const;

private:
  boost::scoped_ptr<OrgFreedesktopUDisksInterface> interface_;

  QMutex mutex_;
  QMap<QString, DeviceData> device_data_;
};

#endif // FILESYSTEMDEVICEENGINE_H
