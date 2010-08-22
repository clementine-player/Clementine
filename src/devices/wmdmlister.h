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

#ifndef WMDMLISTER_H
#define WMDMLISTER_H

#include "devicelister.h"

#include <QMap>
#include <QMutex>
#include <QPixmap>

#include <mswmdm.h>
#include <sac_shim.h>

#undef LoadIcon

class WmdmLister : public DeviceLister, public IWMDMNotification {
  Q_OBJECT

public:
  WmdmLister();

  // DeviceLister
  virtual void Init();

  virtual QStringList DeviceUniqueIDs();
  virtual QVariantList DeviceIcons(const QString& id);
  virtual QString DeviceManufacturer(const QString& id);
  virtual QString DeviceModel(const QString& id);
  virtual quint64 DeviceCapacity(const QString& id);
  virtual quint64 DeviceFreeSpace(const QString& id);
  virtual QVariantMap DeviceHardwareInfo(const QString& id);
  virtual QString MakeFriendlyName(const QString& id);
  virtual QList<QUrl> MakeDeviceUrls(const QString& id);
  virtual void UnmountDevice(const QString& id);

  // IWMDMNotification
  // The __stdcall is *really* important
  virtual HRESULT __stdcall WMDMMessage(DWORD message_type, LPCWSTR name);
  virtual LONG __stdcall QueryInterface(const IID& riid, void** object);
  virtual ULONG __stdcall AddRef();
  virtual ULONG __stdcall Release();

public slots:
  virtual void UpdateDeviceFreeSpace(const QString& id);
  virtual void ShutDown();

private:
  struct DeviceInfo {
    DeviceInfo() : device_(NULL), storage_(NULL), is_suitable_(false),
                   total_bytes_(0), free_bytes_(0) {}

    QString unique_id() const;

    IWMDMDevice2* device_;
    IWMDMStorage2* storage_;

    bool is_suitable_;

    QString name_;
    QString manufacturer_;
    QString canonical_name_;

    QPixmap icon_;

    quint64 total_bytes_;
    quint64 free_bytes_;
  };

  DeviceInfo ReadDeviceInfo(IWMDMDevice2* device);

  template <typename T>
  T LockAndGetDeviceInfo(const QString& id, T DeviceInfo::*field);

private:
  IWMDeviceManager2* device_manager_;
  SacHandle sac_;
  DWORD notification_cookie_;

  QMutex mutex_;
  QMap<QString, DeviceInfo> devices_;
};

template <typename T>
T WmdmLister::LockAndGetDeviceInfo(const QString& id, T DeviceInfo::*field) {
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id))
    return T();

  return devices_[id].*field;
}

#endif // WMDMLISTER_H
