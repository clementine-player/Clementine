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

#ifndef GIOLISTER_H
#define GIOLISTER_H

#include "devicelister.h"

#include <gio/gio.h>

#include <QMutex>
#include <QStringList>

class GioLister : public DeviceLister {
  Q_OBJECT

public:
  GioLister();
  ~GioLister();

  int priority() const { return 50; }

  QStringList DeviceUniqueIDs();
  QStringList DeviceIcons(const QString& id);
  QString DeviceManufacturer(const QString& id);
  QString DeviceModel(const QString& id);
  quint64 DeviceCapacity(const QString& id);
  quint64 DeviceFreeSpace(const QString& id);
  QVariantMap DeviceHardwareInfo(const QString& id);

  QString MakeFriendlyName(const QString &id);

  QUrl MakeDeviceUrl(const QString &id);

protected:
  void Init();

private:
  struct MountInfo {
    MountInfo() : filesystem_size(0), filesystem_free(0) {}

    QString unique_id() const;

    GMount* mount;
    QString mount_path;
    QString name;
    QStringList icon_names;
    QString uuid;
    quint64 filesystem_size;
    quint64 filesystem_free;
    QString filesystem_type;
  };

  void MountAdded(GMount* mount);
  void MountChanged(GMount* mount);
  void MountRemoved(GMount* mount);

  static void MountAddedCallback(GVolumeMonitor*, GMount*, gpointer);
  static void MountChangedCallback(GVolumeMonitor*, GMount*, gpointer);
  static void MountRemovedCallback(GVolumeMonitor*, GMount*, gpointer);

  static QString ConvertAndFree(char* str);
  static MountInfo ReadMountInfo(GMount* mount);

  // You MUST hold the mutex while calling this function
  QString FindUniqueIdByMount(GMount* mount) const;

  template <typename T>
  T LockAndGetMountInfo(const QString& id, T MountInfo::*field);

private:
  GVolumeMonitor* monitor_;

  QMutex mutex_;
  QMap<QString, MountInfo> mounts_;
};

template <typename T>
T GioLister::LockAndGetMountInfo(const QString& id, T MountInfo::*field) {
  QMutexLocker l(&mutex_);
  if (!mounts_.contains(id))
    return T();

  return mounts_[id].*field;
}

#endif // GIOLISTER_H
