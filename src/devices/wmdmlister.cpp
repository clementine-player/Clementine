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

#include "wmdmlister.h"

#include <icomponentauthenticate.h>
#include <objbase.h>
#include <sac_shim.h>
#include <mswmdm.h>
#include <mswmdm_i.c>

#include <boost/bind.hpp>

#include <QPixmap>
#include <QStringList>
#include <QtDebug>

BYTE abPVK[] = {0x00};
BYTE abCert[] = {0x00};

QString WmdmLister::DeviceInfo::unique_id() const {
  // TODO: Serial number?
  return name_;
}

WmdmLister::WmdmLister()
  : device_manager_(NULL)
{
}

void WmdmLister::Init() {
  // Initialise COM
  CoInitialize(0);

  // Authenticate with WMDM
  IComponentAuthenticate* auth;
  if (CoCreateInstance(CLSID_MediaDevMgr, NULL, CLSCTX_ALL,
                       IID_IComponentAuthenticate, (void**) &auth)) {
    qWarning() << "Error creating the IComponentAuthenticate interface";
    return;
  }

  SacHandle sac = CSecureChannelClient_New();
  if (CSecureChannelClient_SetCertificate(
      sac, SAC_CERT_V1, abCert, sizeof(abCert), abPVK, sizeof(abPVK))) {
    qWarning() << "Error setting SAC certificate";
    return;
  }

  CSecureChannelClient_SetInterface(sac, auth);
  if (CSecureChannelClient_Authenticate(sac, SAC_PROTOCOL_V1)) {
    qWarning() << "Error authenticating with SAC";
    return;
  }

  // Create the device manager
  if (auth->QueryInterface(IID_IWMDeviceManager, (void**)&device_manager_)) {
    qWarning() << "Error creating WMDM device manager";
    return;
  }

  // Fetch the inital list of devices
  IWMDMEnumDevice* device_it = NULL;
  if (device_manager_->EnumDevices(&device_it)) {
    qWarning() << "Error querying WMDM devices";
    return;
  }

  // Iterate through the devices
  QMap<QString, DeviceInfo> devices;
  forever {
    IWMDMDevice* device = NULL;
    ULONG fetched = 0;
    if (device_it->Next(1, &device, &fetched) || fetched != 1)
      break;

    DeviceInfo info = ReadDeviceInfo(device);
    if (info.is_suitable_)
      devices[info.unique_id()] = info;

    device->Release();
  }
  device_it->Release();

  // Update the internal cache
  {
    QMutexLocker l(&mutex_);
    devices_ = devices;
  }

  // Notify about the changes
  foreach (const QString& id, devices.keys()) {
    emit DeviceAdded(id);
  }
}

template <typename F>
qint64 GetSpaceValue(F f) {
  DWORD low, high;
  f(&low, &high);

  return (qint64)high << 32 | (qint64)low;
}

WmdmLister::DeviceInfo WmdmLister::ReadDeviceInfo(IWMDMDevice* device) {
  DeviceInfo ret;
  ret.device_ = device;

  // Get text strings
  wchar_t buf[MAX_PATH];
  device->GetName(buf, MAX_PATH);
  ret.name_ = QString::fromWCharArray(buf);

  device->GetManufacturer(buf, MAX_PATH);
  ret.manufacturer_ = QString::fromWCharArray(buf);

  // Get the type and check whether it has storage
  DWORD type = 0;
  device->GetType(&type);
  if (type & WMDM_DEVICE_TYPE_STORAGE)
    ret.is_suitable_ = true;

  // Get the icon
  HICON icon;
  device->GetDeviceIcon((ULONG*)&icon);

  ret.icon_ = QPixmap::fromWinHICON(icon);
  DestroyIcon(icon);

  // Get the main (first) storage for the device
  IWMDMEnumStorage* storage_it = NULL;
  if (device->EnumStorage(&storage_it) == S_OK && storage_it) {
    ULONG storage_fetched = 0;
    if (storage_it->Next(1, &ret.storage_, &storage_fetched) == S_OK) {
      // Get free space information
      IWMDMStorageGlobals* globals;
      ret.storage_->GetStorageGlobals(&globals);

      ret.total_bytes_ = GetSpaceValue(boost::bind(&IWMDMStorageGlobals::GetTotalSize, globals, _1, _2));
      ret.free_bytes_  = GetSpaceValue(boost::bind(&IWMDMStorageGlobals::GetTotalFree, globals, _1, _2));
      ret.free_bytes_ -= GetSpaceValue(boost::bind(&IWMDMStorageGlobals::GetTotalBad,  globals, _1, _2));

      globals->Release();
    }
    storage_it->Release();
  }

  return ret;
}

QStringList WmdmLister::DeviceUniqueIDs() {
  QMutexLocker l(&mutex_);
  return devices_.keys();
}

QVariantList WmdmLister::DeviceIcons(const QString& id) {
  QPixmap pixmap = LockAndGetDeviceInfo(id, &DeviceInfo::icon_);

  if (pixmap.isNull())
    return QVariantList();
  return QVariantList() << pixmap;
}

QString WmdmLister::DeviceManufacturer(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::manufacturer_);
}

QString WmdmLister::DeviceModel(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::name_);
}

quint64 WmdmLister::DeviceCapacity(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::total_bytes_);
}

quint64 WmdmLister::DeviceFreeSpace(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::free_bytes_);
}

QVariantMap WmdmLister::DeviceHardwareInfo(const QString& id) {
  return QVariantMap();
}

QString WmdmLister::MakeFriendlyName(const QString& id) {
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id))
    return QString();

  const DeviceInfo& info = devices_[id];
  if (info.manufacturer_.isEmpty() || info.manufacturer_ == "Unknown")
    return info.name_;

  return info.manufacturer_ + " " + info.name_;
}

QList<QUrl> WmdmLister::MakeDeviceUrls(const QString& id) {
  return QList<QUrl>();
}

void WmdmLister::UnmountDevice(const QString& id) {
}

void WmdmLister::UpdateDeviceFreeSpace(const QString& id) {
}
