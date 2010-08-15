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

struct IWMDMDevice;
struct IWMDeviceManager;

#include <QMap>
#include <QMutex>
#include <QPixmap>

class WmdmLister : public DeviceLister {
  Q_OBJECT

public:
  WmdmLister();

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

public slots:
  virtual void UpdateDeviceFreeSpace(const QString& id);

private:
  struct DeviceInfo {
    DeviceInfo() : is_suitable_(false) {}

    QString unique_id() const;

    bool is_suitable_;

    QString name_;
    QString manufacturer_;

    QPixmap icon_;
  };

  DeviceInfo ReadDeviceInfo(IWMDMDevice* device);

  template <typename T>
  T LockAndGetDeviceInfo(const QString& id, T DeviceInfo::*field);

private:
  IWMDeviceManager* device_manager_;

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
