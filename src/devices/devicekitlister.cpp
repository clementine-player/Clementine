/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "config.h"
#include "devicekitlister.h"
#include "filesystemdevice.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "dbus/udisks.h"
#include "dbus/udisksdevice.h"

#ifdef HAVE_LIBGPOD
#include "gpoddevice.h"
#endif

#include <sys/statvfs.h>

#include <QtDebug>

DeviceKitLister::DeviceKitLister() {}

DeviceKitLister::~DeviceKitLister() { qLog(Debug) << __PRETTY_FUNCTION__; }

QString DeviceKitLister::DeviceData::unique_id() const {
  return QString("DeviceKit/%1/%2/%3/%4")
      .arg(drive_serial, drive_vendor, drive_model)
      .arg(device_size);
}

void DeviceKitLister::Init() {
  interface_.reset(new OrgFreedesktopUDisksInterface(
      OrgFreedesktopUDisksInterface::staticInterfaceName(),
      "/org/freedesktop/UDisks", QDBusConnection::systemBus()));

  // Get all the devices currently attached
  QDBusPendingReply<QList<QDBusObjectPath> > reply =
      interface_->EnumerateDevices();
  reply.waitForFinished();

  if (!reply.isValid()) {
    qLog(Warning) << "Error enumerating DeviceKit-disks devices:"
                  << reply.error().name() << reply.error().message();
    interface_.reset();
    return;
  }

  // Listen for changes
  connect(interface_.get(), SIGNAL(DeviceAdded(QDBusObjectPath)),
          SLOT(DBusDeviceAdded(QDBusObjectPath)));
  connect(interface_.get(), SIGNAL(DeviceRemoved(QDBusObjectPath)),
          SLOT(DBusDeviceRemoved(QDBusObjectPath)));
  connect(interface_.get(), SIGNAL(DeviceChanged(QDBusObjectPath)),
          SLOT(DBusDeviceChanged(QDBusObjectPath)));

  // Get information about each one
  QMap<QString, DeviceData> device_data;
  for (const QDBusObjectPath& path : reply.value()) {
    DeviceData data = ReadDeviceData(path);
    if (data.suitable) device_data[data.unique_id()] = data;
  }

  // Update the internal cache
  {
    QMutexLocker l(&mutex_);
    device_data_ = device_data;
  }

  // Notify about the changes
  for (const QString& id : device_data.keys()) {
    emit DeviceAdded(id);
  }
}

QStringList DeviceKitLister::DeviceUniqueIDs() {
  QMutexLocker l(&mutex_);
  return device_data_.keys();
}

QVariantList DeviceKitLister::DeviceIcons(const QString& id) {
  QString path = LockAndGetDeviceInfo(id, &DeviceData::device_mount_paths)[0];
  return QVariantList() << GuessIconForPath(path)
                        << GuessIconForModel(DeviceManufacturer(id),
                                             DeviceModel(id))
                        << LockAndGetDeviceInfo(
                               id, &DeviceData::device_presentation_icon_name);
}

QString DeviceKitLister::DeviceManufacturer(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceData::drive_vendor);
}

QString DeviceKitLister::DeviceModel(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceData::drive_model);
}

quint64 DeviceKitLister::DeviceCapacity(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceData::device_size);
}

quint64 DeviceKitLister::DeviceFreeSpace(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceData::free_space);
}

QVariantMap DeviceKitLister::DeviceHardwareInfo(const QString& id) {
  QVariantMap ret;

  QMutexLocker l(&mutex_);
  if (!device_data_.contains(id)) return ret;
  const DeviceData& data = device_data_[id];

  ret[QT_TR_NOOP("D-Bus path")] = data.dbus_path;
  ret[QT_TR_NOOP("Serial number")] = data.drive_serial;
  ret[QT_TR_NOOP("Mount points")] = data.device_mount_paths.join(", ");
  ret[QT_TR_NOOP("Device")] = data.device_file;
  return ret;
}

QString DeviceKitLister::MakeFriendlyName(const QString& id) {
  QMutexLocker l(&mutex_);
  if (!device_data_.contains(id)) return QString();
  const DeviceData& data = device_data_[id];

  if (!data.device_presentation_name.isEmpty())
    return data.device_presentation_name;
  if (!data.drive_model.isEmpty() && !data.drive_vendor.isEmpty())
    return data.drive_vendor + " " + data.drive_model;
  if (!data.drive_model.isEmpty()) return data.drive_model;
  return data.drive_serial;
}

DeviceKitLister::DeviceData DeviceKitLister::ReadDeviceData(
    const QDBusObjectPath& path) const {
  DeviceData ret;

  OrgFreedesktopUDisksDeviceInterface device(
      OrgFreedesktopUDisksInterface::staticInterfaceName(), path.path(),
      QDBusConnection::systemBus());
  if (!device.isValid()) {
    qLog(Warning) << "Error connecting to the device interface on"
                  << path.path();
    return ret;
  }

  // Don't do anything with internal drives, hidden drives, or partition tables
  if (device.deviceIsSystemInternal() || device.devicePresentationHide() ||
      device.deviceMountPaths().isEmpty() || device.deviceIsPartitionTable()) {
    return ret;
  }

  ret.suitable = true;
  ret.dbus_path = path.path();
  ret.drive_serial = device.driveSerial();
  ret.drive_model = device.driveModel();
  ret.drive_vendor = device.driveVendor();
  ret.device_file = device.deviceFile();
  ret.device_presentation_name = device.devicePresentationName();
  ret.device_presentation_icon_name = device.devicePresentationIconName();
  ret.device_size = device.deviceSize();
  ret.device_mount_paths = device.deviceMountPaths();

  // Get free space info
  if (!ret.device_mount_paths.isEmpty())
    ret.free_space = Utilities::FileSystemFreeSpace(ret.device_mount_paths[0]);

  return ret;
}

void DeviceKitLister::DBusDeviceAdded(const QDBusObjectPath& path) {
  DeviceData data = ReadDeviceData(path);
  if (!data.suitable) return;

  {
    QMutexLocker l(&mutex_);
    device_data_[data.unique_id()] = data;
  }

  emit DeviceAdded(data.unique_id());
}

void DeviceKitLister::DBusDeviceRemoved(const QDBusObjectPath& path) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByPath(path);
    if (id.isNull()) return;

    device_data_.remove(id);
  }

  emit DeviceRemoved(id);
}

void DeviceKitLister::DBusDeviceChanged(const QDBusObjectPath& path) {
  bool already_known = false;
  {
    QMutexLocker l(&mutex_);
    already_known = !FindUniqueIdByPath(path).isNull();
  }

  DeviceData data = ReadDeviceData(path);

  if (already_known && !data.suitable)
    DBusDeviceRemoved(path);
  else if (!already_known && data.suitable)
    DBusDeviceAdded(path);
  else if (already_known && data.suitable) {
    {
      QMutexLocker l(&mutex_);
      device_data_[data.unique_id()] = data;
    }
    emit DeviceChanged(data.unique_id());
  }
}

QString DeviceKitLister::FindUniqueIdByPath(const QDBusObjectPath& path) const {
  for (const DeviceData& data : device_data_) {
    if (data.dbus_path == path.path()) return data.unique_id();
  }
  return QString();
}

QList<QUrl> DeviceKitLister::MakeDeviceUrls(const QString& id) {
  QString mount_point =
      LockAndGetDeviceInfo(id, &DeviceData::device_mount_paths)[0];

  return QList<QUrl>() << MakeUrlFromLocalPath(mount_point);
}

void DeviceKitLister::UnmountDevice(const QString& id) {
  QString path = LockAndGetDeviceInfo(id, &DeviceData::dbus_path);

  OrgFreedesktopUDisksDeviceInterface device(
      OrgFreedesktopUDisksInterface::staticInterfaceName(), path,
      QDBusConnection::systemBus());
  if (!device.isValid()) {
    qLog(Warning) << "Error connecting to the device interface on" << path;
    return;
  }

  // Get the device's parent drive
  QString drive_path = device.partitionSlave().path();
  OrgFreedesktopUDisksDeviceInterface drive(
      OrgFreedesktopUDisksInterface::staticInterfaceName(), drive_path,
      QDBusConnection::systemBus());
  if (!drive.isValid()) {
    qLog(Warning) << "Error connecting to the drive interface on" << drive_path;
    return;
  }

  // Unmount the filesystem
  QDBusPendingReply<> reply = device.FilesystemUnmount(QStringList());
  reply.waitForFinished();

  // Eject the drive
  drive.DriveEject(QStringList());
  // Don't bother waiting for the eject to finish
}

void DeviceKitLister::UpdateDeviceFreeSpace(const QString& id) {
  {
    QMutexLocker l(&mutex_);
    if (!device_data_.contains(id)) return;

    DeviceData& data = device_data_[id];
    if (!data.device_mount_paths.isEmpty())
      data.free_space =
          Utilities::FileSystemFreeSpace(data.device_mount_paths[0]);
  }

  emit DeviceChanged(id);
}
