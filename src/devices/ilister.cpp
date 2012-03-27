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
#include "ilister.h"
#include "imobiledeviceconnection.h"

#include <QStringList>
#include <QtDebug>

iLister::iLister() {
}

iLister::~iLister() {
}

void iLister::Init() {
  idevice_event_subscribe(&EventCallback, reinterpret_cast<void*>(this));
}

void iLister::EventCallback(const idevice_event_t* event, void* context) {
  iLister* me = reinterpret_cast<iLister*>(context);

#ifdef IMOBILEDEVICE_USES_UDIDS
  const char* uuid = event->udid;
#else
  const char* uuid = event->uuid;
#endif

  switch (event->event) {
    case IDEVICE_DEVICE_ADD:
      me->DeviceAddedCallback(uuid);
      break;

    case IDEVICE_DEVICE_REMOVE:
      me->DeviceRemovedCallback(uuid);
      break;
  }
}


void iLister::DeviceAddedCallback(const char* uuid) {
  DeviceInfo info = ReadDeviceInfo(uuid);
  QString id = UniqueId(uuid);

  QString name = MakeFriendlyName(id);
  if (info.product_type == "iPhone 3,1" || info.product_type.startsWith("iPad")) {
    // iPhone 4 and iPad unsupported by libgpod as of 0.7.94.
    return;
  }

  {
    QMutexLocker l(&mutex_);
    devices_[id] = info;
  }

  emit DeviceAdded(id);
}

void iLister::DeviceRemovedCallback(const char* uuid) {
  QString id = UniqueId(uuid);
  {
    QMutexLocker l(&mutex_);
    if (!devices_.contains(id))
      return;

    devices_.remove(id);
  }

  emit DeviceRemoved(id);
}

QString iLister::UniqueId(const char *uuid) {
  return "ithing/" + QString::fromUtf8(uuid);
}

QStringList iLister::DeviceUniqueIDs() {
  return devices_.keys();
}

QVariantList iLister::DeviceIcons(const QString& id) {
  return QVariantList() << "ipodtouchicon";
}

QString iLister::DeviceManufacturer(const QString& id) {
  return "Apple";
}

QString iLister::DeviceModel(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::product_type);
}

quint64 iLister::DeviceCapacity(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::total_bytes);
}

quint64 iLister::DeviceFreeSpace(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::free_bytes);
}

QVariantMap iLister::DeviceHardwareInfo(const QString& id) {
  QVariantMap ret;
  ret[tr("Color")] = LockAndGetDeviceInfo(id, &DeviceInfo::colour);
  ret["IMEI"] = LockAndGetDeviceInfo(id, &DeviceInfo::imei);
  ret[tr("Password Protected")] = LockAndGetDeviceInfo(id, &DeviceInfo::password_protected);
  ret[tr("Timezone")] = LockAndGetDeviceInfo(id, &DeviceInfo::timezone);
  ret[tr("WiFi MAC Address")] = LockAndGetDeviceInfo(id, &DeviceInfo::wifi_mac);
  ret[tr("Bluetooth MAC Address")] = LockAndGetDeviceInfo(id, &DeviceInfo::bt_mac);

  return ret;
}

QString iLister::MakeFriendlyName(const QString& id) {
  QString name = LockAndGetDeviceInfo(id, &DeviceInfo::name);
  if (!name.isEmpty()) {
    return name;
  }

  QString model_id = LockAndGetDeviceInfo(id, &DeviceInfo::product_type);

  if (model_id.startsWith("iPhone")) {
    QString version = model_id.right(3);
    QChar major = version[0];
    QChar minor = version[2];
    if (major == '1' && minor == '1') {
      return "iPhone";
    }
    if (major == '1' && minor == '2') {
      return "iPhone 3G";
    }
    if (major == '2' && minor == '1') {
      return "iPhone 3GS";
    }
    if (major == '3' && minor == '1') {
      return "iPhone 4";
    }
  } else if (model_id.startsWith("iPod")) {
    return "iPod Touch";
  } else if (model_id.startsWith("iPad")) {
    return "iPad";
  }
  return model_id;
}

QList<QUrl> iLister::MakeDeviceUrls(const QString& id) {
  QList<QUrl> ret;

  QString uuid = LockAndGetDeviceInfo(id, &DeviceInfo::uuid);
  if (uuid.isEmpty())
    return ret;

  ret << QUrl("afc://" + uuid + "/");

  return ret;
}

void iLister::UnmountDevice(const QString& id) { }

iLister::DeviceInfo iLister::ReadDeviceInfo(const char* uuid) {
  DeviceInfo ret;

  iMobileDeviceConnection conn(uuid);
  ret.uuid = uuid;
  ret.product_type = conn.GetProperty("ProductType").toString();
  ret.free_bytes = conn.GetProperty("AmountDataAvailable", "com.apple.disk_usage").toULongLong();
  ret.total_bytes = conn.GetProperty("TotalDataCapacity", "com.apple.disk_usage").toULongLong();
  ret.name = conn.GetProperty("DeviceName").toString();

  ret.colour = conn.GetProperty("DeviceColor").toString();
  ret.imei = conn.GetProperty("InternationalMobileEquipmentIdentity").toString();
  ret.hardware = conn.GetProperty("HardwareModel").toString();
  ret.password_protected = conn.GetProperty("PasswordProtected").toBool();
  ret.os_version = conn.GetProperty("ProductVersion").toString();
  ret.timezone = conn.GetProperty("TimeZone").toString();
  ret.wifi_mac = conn.GetProperty("WiFiAddress").toString();
  ret.bt_mac = conn.GetProperty("BluetoothAddress").toString();

  return ret;
}

void iLister::UpdateDeviceFreeSpace(const QString& id) {
  {
    QMutexLocker l(&mutex_);
    if (!devices_.contains(id))
      return;

    DeviceInfo& info = devices_[id];
    iMobileDeviceConnection conn(info.uuid);

    info.free_bytes = conn.GetProperty("AmountDataAvailable", "com.apple.disk_usage").toULongLong();
  }

  emit DeviceChanged(id);
}
