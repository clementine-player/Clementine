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

#define _WIN32_WINNT 0x0501

#include "wmdmlister.h"
#include "wmdmthread.h"
#include "core/utilities.h"

#include <objbase.h>
#include <mswmdm_i.c>
#include <winbase.h>

#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>

#include <QPixmap>
#include <QStringList>
#include <QtDebug>

const QUuid WmdmLister::kDeviceProtocolMsc(
    0xa4d2c26c, 0xa881, 0x44bb, 0xbd, 0x5d, 0x1f, 0x70, 0x3c, 0x71, 0xf7, 0xa9);

QString WmdmLister::CanonicalNameToId(const QString& canonical_name) {
  return "wmdm/" + canonical_name;
}

QString WmdmLister::DeviceInfo::unique_id() const {
  return WmdmLister::CanonicalNameToId(canonical_name_);
}

WmdmLister::WmdmLister()
  : notification_cookie_(0)
{
}

WmdmLister::~WmdmLister() {
  Q_ASSERT(!thread_);
}

void WmdmLister::Init() {
  thread_.reset(new WmdmThread);

  // Register for notifications
  IConnectionPointContainer* cp_container = NULL;
  thread_->manager()->QueryInterface(IID_IConnectionPointContainer, (void**)&cp_container);

  IConnectionPoint* cp = NULL;
  cp_container->FindConnectionPoint(IID_IWMDMNotification, &cp);

  cp->Advise(this, &notification_cookie_);

  cp->Release();
  cp_container->Release();

  // Fetch the initial list of devices
  IWMDMEnumDevice* device_it = NULL;
  if (thread_->manager()->EnumDevices2(&device_it)) {
    qWarning() << "Error querying WMDM devices";
    return;
  }

  // Iterate through the devices
  QMap<QString, DeviceInfo> devices;
  forever {
    IWMDMDevice* device = NULL;
    IWMDMDevice2* device2 = NULL;
    ULONG fetched = 0;
    if (device_it->Next(1, &device, &fetched) || fetched != 1)
      break;

    if (device->QueryInterface(IID_IWMDMDevice2, (void**)&device2)) {
      qWarning() << "Error getting IWMDMDevice2 from device";
      device->Release();
      continue;
    }
    device->Release();

    DeviceInfo info = ReadDeviceInfo(device2);
    if (info.is_suitable_)
      devices[info.unique_id()] = info;
    else
      device2->Release();
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

void WmdmLister::ReallyShutdown() {
  // Unregister for notifications
  IConnectionPointContainer* cp_container;
  thread_->manager()->QueryInterface(IID_IConnectionPointContainer, (void**)&cp_container);

  IConnectionPoint* cp;
  cp_container->FindConnectionPoint(IID_IWMDMNotification, &cp);

  cp->Release();
  cp_container->Release();

  thread_.reset();
}

void WmdmLister::ShutDown() {
  // COM shutdown must be done in the original thread.
  metaObject()->invokeMethod(this, "ReallyShutdown", Qt::BlockingQueuedConnection);
}

template <typename F>
qint64 GetSpaceValue(F f) {
  DWORD low, high;
  f(&low, &high);

  return (qint64)high << 32 | (qint64)low;
}

WmdmLister::DeviceInfo WmdmLister::ReadDeviceInfo(IWMDMDevice2* device) {
  DeviceInfo ret;
  ret.device_ = device;

  // Get text strings
  const int max_size = 512;
  wchar_t buf[max_size];
  device->GetName(buf, max_size);
  ret.name_ = QString::fromWCharArray(buf).trimmed();

  device->GetManufacturer(buf, max_size);
  ret.manufacturer_ = QString::fromWCharArray(buf).trimmed();

  device->GetCanonicalName(buf, max_size);
  ret.canonical_name_ = QString::fromWCharArray(buf).toLower();

  // Upgrade to a device3
  IWMDMDevice3* device3 = NULL;
  device->QueryInterface(IID_IWMDMDevice3, (void**)&device3);

  // Get the device protocol so we can figure out whether the device is MSC
  PROPVARIANT protocol;
  if (device3) {
    device3->GetProperty(g_wszWMDMDeviceProtocol, &protocol);
    device3->Release();
  }

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
    IWMDMStorage* storage;

    if (storage_it->Next(1, &storage, &storage_fetched) == S_OK) {
      if (storage->QueryInterface(IID_IWMDMStorage2, (void**)&ret.storage_)) {
        qWarning() << "Error getting IWMDMStorage2 from storage";
      } else {
        // Get free space information
        UpdateFreeSpace(&ret);
      }
      storage->Release();
    }
    storage_it->Release();
  }

  // There doesn't seem to be a way to get the drive letter of MSC devices, so
  // try parsing the device's name to extract it.
  if (!device3 || QUuid(*protocol.puuid) == kDeviceProtocolMsc)
    GuessDriveLetter(&ret);

  return ret;
}

void WmdmLister::GuessDriveLetter(DeviceInfo* info) {
  qDebug() << "Guessing drive letter for" << info->name_;

  // Windows XP puts the drive letter in brackets at the end of the name
  QRegExp drive_letter("\\(([A-Z]:)\\)$");
  if (drive_letter.indexIn(info->name_) != -1) {
    qDebug() << "Looks like an XP drive" << drive_letter.cap(1);
    CheckDriveLetter(info, drive_letter.cap(1));
    return;
  }

  // Windows 7 sometimes has the drive letter as the whole name
  drive_letter = QRegExp("^([A-Z]:)\\\\$");
  if (drive_letter.indexIn(info->name_) != -1) {
    qDebug() << "Looks like a win7 drive" << drive_letter.cap(1);
    CheckDriveLetter(info, drive_letter.cap(1));
    return;
  }

  // Otherwise Windows 7 uses the drive's DOS label as its whole name.
  // Let's enumerate all the volumes on the system and find one with that
  // label, then get its drive letter.  Yay!
  wchar_t volume_name[MAX_PATH + 1];
  HANDLE handle = FindFirstVolumeW(volume_name, MAX_PATH);

  forever {
    // QueryDosDeviceW doesn't allow a trailing backslash, so remove it.
    int length = wcslen(volume_name);
    volume_name[length - 1] = L'\0';

    wchar_t device_name[MAX_PATH + 1];
    QueryDosDeviceW(&volume_name[4], device_name, MAX_PATH);

    volume_name[length - 1] = L'\\';

    // Don't do cd-roms or floppies
    if (QString::fromWCharArray(device_name).contains("HarddiskVolume")) {
      wchar_t volume_path[MAX_PATH + 1];
      DWORD volume_path_length = MAX_PATH;
      GetVolumePathNamesForVolumeNameW(
          volume_name, volume_path, volume_path_length, &volume_path_length);

      if (wcslen(volume_path) == 3) {
        ScopedWCharArray name(QString(MAX_PATH + 1, '\0'));
        ScopedWCharArray type(QString(MAX_PATH + 1, '\0'));
        DWORD serial = 0;

        if (!GetVolumeInformationW(volume_path, name, MAX_PATH,
            &serial, NULL, NULL, type, MAX_PATH)) {
          qWarning() << "Error getting volume information for" <<
              QString::fromWCharArray(volume_path);
        } else {
          if (name.ToString() == info->name_ && name.characters() != 0) {
            // We found it!
            qDebug() << "Looks like a win7 drive name" << QString::fromWCharArray(volume_path);
            CheckDriveLetter(info, QString::fromWCharArray(volume_path));
            break;
          }
        }
      }
    }

    if (!FindNextVolumeW(handle, volume_name, MAX_PATH))
      break;
  }
  FindVolumeClose(handle);
}

void WmdmLister::CheckDriveLetter(DeviceInfo* info, const QString& drive) {
  // Sanity check to make sure there really is a drive there
  ScopedWCharArray path(drive.endsWith('\\') ? drive : (drive + "\\"));
  ScopedWCharArray name(QString(MAX_PATH + 1, '\0'));
  ScopedWCharArray type(QString(MAX_PATH + 1, '\0'));
  DWORD serial = 0;

  if (!GetVolumeInformationW(
      path,
      name, MAX_PATH,
      &serial,
      NULL, // max component length
      NULL, // flags
      type, MAX_PATH // fat or ntfs
      )) {
    qWarning() << "Error getting volume information for" << drive;
  } else {
    qDebug() << "Validated drive letter" << drive;
    info->mount_point_ = drive + "/";
    info->fs_name_ = name.ToString();
    info->fs_type_ = type.ToString();
    info->fs_serial_ = serial;
  }
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
  QList<QUrl> ret;

  QString mount_point = LockAndGetDeviceInfo(id, &DeviceInfo::mount_point_);
  if (!mount_point.isEmpty()) {
    ret << MakeUrlFromLocalPath(mount_point);
  }

  QUrl wmdm_url;
  wmdm_url.setScheme("wmdm");
  wmdm_url.setPath(id);
  ret << wmdm_url;

  return ret;
}

void WmdmLister::UnmountDevice(const QString& id) {
}

void WmdmLister::UpdateDeviceFreeSpace(const QString& id) {
  // This needs to be done in the lister's thread where we already have COM
  // initialised
  metaObject()->invokeMethod(this, "DoUpdateDriveFreeSpace",
                             Qt::BlockingQueuedConnection, Q_ARG(QString, id));
}

void WmdmLister::DoUpdateDriveFreeSpace(const QString& id) {
  {
    QMutexLocker l(&mutex_);
    if (!devices_.contains(id))
      return;

    UpdateFreeSpace(&devices_[id]);
  }

  emit DeviceChanged(id);
}

void WmdmLister::UpdateFreeSpace(DeviceInfo* info) {
  IWMDMStorageGlobals* globals;
  info->storage_->GetStorageGlobals(&globals);

  info->total_bytes_ = GetSpaceValue(boost::bind(&IWMDMStorageGlobals::GetTotalSize, globals, _1, _2));
  info->free_bytes_  = GetSpaceValue(boost::bind(&IWMDMStorageGlobals::GetTotalFree, globals, _1, _2));
  info->free_bytes_ -= GetSpaceValue(boost::bind(&IWMDMStorageGlobals::GetTotalBad,  globals, _1, _2));

  globals->Release();
}

HRESULT WmdmLister::WMDMMessage(DWORD message_type, LPCWSTR name) {
  QString canonical_name = QString::fromWCharArray(name).toLower();

  switch (message_type) {
    case WMDM_MSG_DEVICE_ARRIVAL: WMDMDeviceAdded(canonical_name);   break;
    case WMDM_MSG_DEVICE_REMOVAL: WMDMDeviceRemoved(canonical_name); break;
  }

  return S_OK;
}

void WmdmLister::WMDMDeviceAdded(const QString& canonical_name) {
  ScopedWCharArray name(canonical_name);

  IWMDMDevice* device = NULL;
  if (thread_->manager()->GetDeviceFromCanonicalName(name, &device)) {
    qWarning() << "Error in GetDeviceFromCanonicalName for" << canonical_name;
    return;
  }

  IWMDMDevice2* device2 = NULL;
  if (device->QueryInterface(IID_IWMDMDevice2, (void**) &device2)) {
    qWarning() << "Error getting IWMDMDevice2 from device";
    device->Release();
    return;
  }
  device->Release();

  DeviceInfo info = ReadDeviceInfo(device2);
  if (info.is_suitable_) {
    QString id = info.unique_id();
    {
      QMutexLocker l(&mutex_);
      devices_[id] = info;
    }
    emit DeviceAdded(id);
  } else {
    device2->Release();
  }
}

void WmdmLister::WMDMDeviceRemoved(const QString& canonical_name) {
  QString id = CanonicalNameToId(canonical_name);
  {
    QMutexLocker l(&mutex_);
    if (!devices_.contains(id))
      return;

    devices_[id].device_->Release();
    devices_[id].storage_->Release();

    devices_.remove(id);
  }

  emit DeviceRemoved(id);
}

LONG WmdmLister::QueryInterface(REFIID riid, void** object) {
  *object = 0;

  if (riid == IID_IUnknown)
    *object = (IUnknown*) this;
  else if (riid == IID_IWMDMNotification)
    *object = (IWMDMNotification*) this;
  else
    return E_NOINTERFACE;

  return S_OK;
}

ULONG WmdmLister::AddRef() {
  return 0;
}

ULONG WmdmLister::Release() {
  return 0;
}

QString WmdmLister::DeviceCanonicalName(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::canonical_name_);
}

