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

class GioLister : public DeviceLister {
  Q_OBJECT

public:
  GioLister();
  ~GioLister();

  QStringList DeviceUniqueIDs();
  QString DeviceIcon(const QString& id);
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
  void MountAdded(GMount* mount);
  void MountChanged(GMount* mount);
  void MountRemoved(GMount* mount);

  static void MountAddedCallback(GVolumeMonitor*, GMount*, gpointer);
  static void MountChangedCallback(GVolumeMonitor*, GMount*, gpointer);
  static void MountRemovedCallback(GVolumeMonitor*, GMount*, gpointer);

private:
  GVolumeMonitor* monitor_;
};

#endif // GIOLISTER_H
