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

#include "filesystemdeviceengine.h"
#include "dbus/udisks.h"
#include "dbus/udisksdevice.h"

FilesystemDeviceEngine::FilesystemDeviceEngine(QObject *parent)
  : DeviceEngine(parent)
{
}

FilesystemDeviceEngine::~FilesystemDeviceEngine() {
}

bool FilesystemDeviceEngine::Init() {
  interface_.reset(new OrgFreedesktopUDisksInterface(
      OrgFreedesktopUDisksInterface::staticInterfaceName(),
      "/org/freedesktop/UDisks", QDBusConnection::systemBus()));

  if (!interface_->isValid()) {
    qWarning() << "Error connecting to the DeviceKit-disks DBUS service";
    interface_.reset();
    return false;
  }

  connect(interface_.get(), SIGNAL(DeviceAdded(QDBusObjectPath)), SLOT(DeviceAdded(QDBusObjectPath)));
  connect(interface_.get(), SIGNAL(DeviceRemoved(QDBusObjectPath)), SLOT(DeviceRemoved(QDBusObjectPath)));
  connect(interface_.get(), SIGNAL(DeviceChanged(QDBusObjectPath)), SLOT(DeviceChanged(QDBusObjectPath)));

  Reset();
  return true;
}

QModelIndex FilesystemDeviceEngine::index(int row, int column, const QModelIndex &parent) const {
  if (parent.isValid())
    return QModelIndex();
  return createIndex(row, column);
}

int FilesystemDeviceEngine::rowCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return device_info_.count();
}

int FilesystemDeviceEngine::columnCount(const QModelIndex &parent) const {
  return LastFilesystemDeviceEngineColumn;
}

QVariant FilesystemDeviceEngine::data(const QModelIndex &index, int role) const {
  const DeviceInfo& info = device_info_[index.row()];

  switch (index.column()) {
    case Column_UniqueID:
      return info.unique_id();

    case Column_FriendlyName:
      if (!info.device_presentation_name.isEmpty())
        return info.device_presentation_name;
      if (!info.drive_model.isEmpty() || !info.drive_vendor.isEmpty())
        return QString("%1 %2").arg(info.drive_vendor, info.drive_model);
      return info.drive_serial;

    case Column_Manufacturer:
      return info.drive_vendor;

    case Column_Model:
      return info.drive_model;

    case Column_Capacity:
      return info.device_size;

    case Column_FreeSpace:
      return QVariant();

    case Column_DbusPath:
      return info.dbus_path;

    case Column_MountPath:
      return info.device_mount_paths.isEmpty() ? QVariant() : info.device_mount_paths[0];

    default:
      return QVariant();
  }
}

void FilesystemDeviceEngine::Reset() {
  QDBusPendingReply<QList<QDBusObjectPath> > reply = interface_->EnumerateDevices();
  reply.waitForFinished();

  if (!reply.isValid()) {
    qWarning() << "Error enumerating DeviceKit-disks devices:" << reply.error();
    return;
  }

#if QT_VERSION >= 0x040600
  emit beginResetModel();
#endif

  device_info_.clear();
  foreach (const QDBusObjectPath& path, reply.value()) {
    DeviceInfo info = ReadDeviceInfo(path);
    if (info.suitable)
      device_info_ << info;
  }

#if QT_VERSION >= 0x040600
  emit endResetModel();
#else
  reset();
#endif
}

FilesystemDeviceEngine::DeviceInfo FilesystemDeviceEngine::ReadDeviceInfo(
    const QDBusObjectPath &path) const {
  DeviceInfo ret;

  OrgFreedesktopUDisksDeviceInterface device(
      OrgFreedesktopUDisksDeviceInterface::staticInterfaceName(),
      path.path(), QDBusConnection::systemBus());
  if (!device.isValid()) {
    qWarning() << "Error connecting to the device interface on" << path.path();
    return ret;
  }

  // Don't do anything with internal drives, hidden drives, or things that
  // aren't partitions
  if (device.deviceIsSystemInternal() ||
      device.devicePresentationHide() ||
      !device.deviceIsPartition()) {
    return ret;
  }

  ret.suitable = true;
  ret.dbus_path = path.path();
  ret.drive_serial = device.driveSerial();
  ret.drive_model = device.driveModel();
  ret.drive_vendor = device.driveVendor();
  ret.device_presentation_name = device.devicePresentationName();
  ret.device_presentation_icon_name = device.devicePresentationIconName();
  ret.device_size = device.deviceSize();
  return ret;
}

void FilesystemDeviceEngine::DeviceAdded(const QDBusObjectPath &path) {
  DeviceInfo info = ReadDeviceInfo(path);
  if (!info.suitable)
    return;

  emit beginInsertRows(QModelIndex(), device_info_.count(), device_info_.count());
  device_info_ << info;
  emit endInsertRows();
}

void FilesystemDeviceEngine::DeviceRemoved(const QDBusObjectPath &path) {
  QModelIndex index = FindDevice(path);
  if (!index.isValid())
    return;

  emit beginRemoveRows(QModelIndex(), index.row(), index.row());
  device_info_.removeAt(index.row());
  emit endRemoveRows();
}

void FilesystemDeviceEngine::DeviceChanged(const QDBusObjectPath &path) {
  QModelIndex index = FindDevice(path);
  DeviceInfo info = ReadDeviceInfo(path);

  if (index.isValid() && !info.suitable)
    DeviceRemoved(path);
  else if (!index.isValid() && info.suitable)
    DeviceAdded(path);
  else if (index.isValid() && info.suitable) {
    device_info_[index.row()] = info;
    emit dataChanged(index, index.sibling(index.row(), columnCount()));
  }
}
