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

#include <QStringList>

#include <boost/scoped_ptr.hpp>

class OrgFreedesktopUDisksInterface;

class QDBusObjectPath;

class FilesystemDeviceEngine : public DeviceEngine {
  Q_OBJECT

public:
  FilesystemDeviceEngine(QObject *parent = 0);
  ~FilesystemDeviceEngine();

  enum Column {
    Column_MountPath = LastDeviceEngineColumn,
    Column_DbusPath,

    LastFilesystemDeviceEngineColumn
  };

  bool Init();

  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;

private slots:
  void DeviceAdded(const QDBusObjectPath& path);
  void DeviceRemoved(const QDBusObjectPath& path);
  void DeviceChanged(const QDBusObjectPath& path);

private:
  struct DeviceInfo {
    DeviceInfo() : suitable(false), device_size(0) {}

    bool suitable;
    QString dbus_path;
    QString drive_serial;
    QString drive_model;
    QString drive_vendor;
    QString device_presentation_name;
    QString device_presentation_icon_name;
    QStringList device_mount_paths;
    quint64 device_size;

    QString unique_id() const;
  };

  void Reset();
  DeviceInfo ReadDeviceInfo(const QDBusObjectPath& path) const;

  QModelIndex FindDevice(const QDBusObjectPath& path) const;

private:
  boost::scoped_ptr<OrgFreedesktopUDisksInterface> interface_;

  QList<DeviceInfo> device_info_;
};

#endif // FILESYSTEMDEVICEENGINE_H
