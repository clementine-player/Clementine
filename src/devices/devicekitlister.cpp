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

#include "devicekitlister.h"
#include "filesystemdevice.h"
#include "dbus/udisks.h"
#include "dbus/udisksdevice.h"

#include <QtDebug>

DeviceKitLister::DeviceKitLister()
{
}

DeviceKitLister::~DeviceKitLister() {
  qDebug() << __PRETTY_FUNCTION__;
}

QString DeviceKitLister::DeviceData::unique_id() const {
  return QString("%1 %2 %3 %4").arg(drive_serial, drive_vendor, drive_model).arg(device_size);
}

void DeviceKitLister::Init() {
  interface_.reset(new OrgFreedesktopUDisksInterface(
      OrgFreedesktopUDisksInterface::staticInterfaceName(),
      "/org/freedesktop/UDisks", QDBusConnection::systemBus()));

  if (!interface_->isValid()) {
    qWarning() << "Error connecting to the DeviceKit-disks DBUS service";
    interface_.reset();
    return;
  }

  // Listen for changes
  connect(interface_.get(), SIGNAL(DeviceAdded(QDBusObjectPath)), SLOT(DBusDeviceAdded(QDBusObjectPath)));
  connect(interface_.get(), SIGNAL(DeviceRemoved(QDBusObjectPath)), SLOT(DBusDeviceRemoved(QDBusObjectPath)));
  connect(interface_.get(), SIGNAL(DeviceChanged(QDBusObjectPath)), SLOT(DBusDeviceChanged(QDBusObjectPath)));

  // Get all the devices currently attached
  QDBusPendingReply<QList<QDBusObjectPath> > reply = interface_->EnumerateDevices();
  reply.waitForFinished();

  if (!reply.isValid()) {
    qWarning() << "Error enumerating DeviceKit-disks devices:" << reply.error().name() << reply.error().message();
    return;
  }

  // Get information about each one
  QMap<QString, DeviceData> device_data;
  foreach (const QDBusObjectPath& path, reply.value()) {
    DeviceData data = ReadDeviceData(path);
    if (data.suitable)
      device_data[data.unique_id()] = data;
  }

  // Update the internal cache
  {
    QMutexLocker l(&mutex_);
    device_data_ = device_data;
  }

  // Notify about the changes
  foreach (const QString& id, device_data.keys()) {
    emit DeviceAdded(id);
  }
}

QStringList DeviceKitLister::DeviceUniqueIDs() {
  QMutexLocker l(&mutex_);
  return device_data_.keys();
}

QString DeviceKitLister::DeviceIcon(const QString &id) {
  return LockAndGetDeviceInfo(id, &DeviceData::device_presentation_icon_name);
}

QString DeviceKitLister::DeviceManufacturer(const QString &id) {
  return LockAndGetDeviceInfo(id, &DeviceData::drive_vendor);
}

QString DeviceKitLister::DeviceModel(const QString &id) {
  return LockAndGetDeviceInfo(id, &DeviceData::drive_model);
}

quint64 DeviceKitLister::DeviceCapacity(const QString &id) {
  return LockAndGetDeviceInfo(id, &DeviceData::device_size);
}

quint64 DeviceKitLister::DeviceFreeSpace(const QString &id) {
  return 0; // TODO
}

QVariantMap DeviceKitLister::DeviceHardwareInfo(const QString &id) {
  QVariantMap ret;

  QMutexLocker l(&mutex_);
  if (!device_data_.contains(id))
    return ret;
  const DeviceData& data = device_data_[id];

  ret[QT_TR_NOOP("DBus path")] = data.dbus_path;
  ret[QT_TR_NOOP("Serial number")] = data.drive_serial;
  ret[QT_TR_NOOP("Mount points")] = data.device_mount_paths.join(", ");
  ret[QT_TR_NOOP("Device")] = data.device_file;
  return ret;
}

QString DeviceKitLister::MakeFriendlyName(const QString &id) {
  QMutexLocker l(&mutex_);
  if (!device_data_.contains(id))
    return QString();
  const DeviceData& data = device_data_[id];

  if (!data.device_presentation_name.isEmpty())
    return data.device_presentation_name;
  if (!data.drive_model.isEmpty() && !data.drive_vendor.isEmpty())
    return data.drive_vendor + " " + data.drive_model;
  if (!data.drive_model.isEmpty())
    return data.drive_model;
  return data.drive_serial;
}

DeviceKitLister::DeviceData DeviceKitLister::ReadDeviceData(
    const QDBusObjectPath &path) const {
  DeviceData ret;

  OrgFreedesktopUDisksDeviceInterface device(
      OrgFreedesktopUDisksInterface::staticInterfaceName(),
      path.path(), QDBusConnection::systemBus());
  if (!device.isValid()) {
    qWarning() << "Error connecting to the device interface on" << path.path();
    return ret;
  }

  // Don't do anything with internal drives, hidden drives, or things that
  // aren't partitions
  if (device.deviceIsSystemInternal() ||
      device.devicePresentationHide() ||
      device.deviceMountPaths().isEmpty() ||
      !device.deviceIsPartition()) {
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
  return ret;
}

void DeviceKitLister::DBusDeviceAdded(const QDBusObjectPath &path) {
  DeviceData data = ReadDeviceData(path);
  if (!data.suitable)
    return;

  {
    QMutexLocker l(&mutex_);
    device_data_[data.unique_id()] = data;
  }

  emit DeviceAdded(data.unique_id());
}

void DeviceKitLister::DBusDeviceRemoved(const QDBusObjectPath &path) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByPath(path);
    if (id.isNull())
      return;

    device_data_.remove(id);
  }

  emit DeviceRemoved(id);
}

void DeviceKitLister::DBusDeviceChanged(const QDBusObjectPath &path) {
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

QString DeviceKitLister::FindUniqueIdByPath(const QDBusObjectPath &path) const {
  foreach (const DeviceData& data, device_data_) {
    if (data.dbus_path == path.path())
      return data.unique_id();
  }
  return QString();
}

boost::shared_ptr<ConnectedDevice> DeviceKitLister::Connect(
    const QString &unique_id, DeviceManager* manager, int database_id,
    bool first_time) {
  return boost::shared_ptr<ConnectedDevice>(new FilesystemDevice(
      LockAndGetDeviceInfo(unique_id, &DeviceData::device_mount_paths)[0],
      this, unique_id, manager, database_id, first_time));
}
