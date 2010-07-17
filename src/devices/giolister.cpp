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

#include "giolister.h"

#include <QStringList>
#include <QtDebug>

GioLister::GioLister()
  : monitor_(NULL)
{
}

void GioLister::Init() {
  monitor_ = g_volume_monitor_get();

  // Get things that are already mounted
  GList* mounts = g_volume_monitor_get_mounts(monitor_);
  for (; mounts ; mounts=mounts->next) {
    GMount* mount = static_cast<GMount*>(mounts->data);

    MountAdded(mount);
    g_object_unref(mount);
  }
  g_list_free(mounts);

  // Connect signals from the monitor
  g_signal_connect(monitor_, "mount-added", G_CALLBACK(MountAddedCallback), this);
  g_signal_connect(monitor_, "mount-changed", G_CALLBACK(MountChangedCallback), this);
  g_signal_connect(monitor_, "mount-removed", G_CALLBACK(MountRemovedCallback), this);
}

GioLister::~GioLister() {
  if (monitor_)
    g_object_unref(monitor_);
}

QStringList GioLister::DeviceUniqueIDs() {
  return QStringList();
}

QString GioLister::DeviceIcon(const QString &id) {
  return QString();
}

QString GioLister::DeviceManufacturer(const QString &id) {
  return QString();
}

QString GioLister::DeviceModel(const QString &id) {
  return QString();
}

quint64 GioLister::DeviceCapacity(const QString &id) {
  return 0;
}

quint64 GioLister::DeviceFreeSpace(const QString &id) {
  return 0; // TODO
}

QString GioLister::MakeFriendlyName(const QString &id) {
  return QString();
}

QVariantMap GioLister::DeviceHardwareInfo(const QString &id) {
  return QVariantMap();
}

QUrl GioLister::MakeDeviceUrl(const QString &id) {
  return QUrl();
}

void GioLister::MountAddedCallback(GVolumeMonitor*, GMount* m, gpointer d) {
  static_cast<GioLister*>(d)->MountAdded(m);
}

void GioLister::MountChangedCallback(GVolumeMonitor*, GMount* m, gpointer d) {
  static_cast<GioLister*>(d)->MountChanged(m);
}

void GioLister::MountRemovedCallback(GVolumeMonitor*, GMount* m, gpointer d) {
  static_cast<GioLister*>(d)->MountRemoved(m);
}

void GioLister::MountAdded(GMount *mount) {
  qDebug() << "mount added" << g_mount_get_name(mount);
}

void GioLister::MountChanged(GMount *mount) {
  qDebug() << "mount changed" << g_mount_get_name(mount);
}

void GioLister::MountRemoved(GMount *mount) {
  qDebug() << "mount removed" << g_mount_get_name(mount);
}
