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

#include "config.h"
#include "giolister.h"

#include <QFile>
#include <QStringList>
#include <QtDebug>

#include <boost/bind.hpp>

QString GioLister::MountInfo::unique_id() const {
  return QString("Gio/%1/%2/%3").arg(uuid, filesystem_type).arg(filesystem_size);
}

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
  QMutexLocker l(&mutex_);
  return mounts_.keys();
}

QStringList GioLister::DeviceIcons(const QString &id) {
  QStringList ret;
  QString path = LockAndGetMountInfo(id, &MountInfo::mount_path);
  ret << DeviceLister::GuessIconForPath(path)
      << DeviceLister::GuessIconForModel(DeviceManufacturer(id), DeviceModel(id))
      << LockAndGetMountInfo(id, &MountInfo::icon_names);
  return ret;
}

QString GioLister::DeviceManufacturer(const QString &id) {
  return QString();
}

QString GioLister::DeviceModel(const QString &id) {
  return LockAndGetMountInfo(id, &MountInfo::name);
}

quint64 GioLister::DeviceCapacity(const QString &id) {
  return LockAndGetMountInfo(id, &MountInfo::filesystem_size);
}

quint64 GioLister::DeviceFreeSpace(const QString &id) {
  return LockAndGetMountInfo(id, &MountInfo::filesystem_free);
}

QString GioLister::MakeFriendlyName(const QString &id) {
  return DeviceModel(id);
}

QVariantMap GioLister::DeviceHardwareInfo(const QString &id) {
  QVariantMap ret;

  QMutexLocker l(&mutex_);
  if (!mounts_.contains(id))
    return ret;
  const MountInfo& info = mounts_[id];

  ret[QT_TR_NOOP("Mount point")] = info.mount_path;
  ret[QT_TR_NOOP("Device")] = info.unix_device;
  ret[QT_TR_NOOP("URI")] = info.uri;
  return ret;
}

QUrl GioLister::MakeDeviceUrl(const QString &id) {
  QString mount_point = LockAndGetMountInfo(
      id, &MountInfo::mount_path);

  return MakeUrlFromLocalPath(mount_point);
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
  MountInfo info = ReadMountInfo(mount);

  {
    QMutexLocker l(&mutex_);
    mounts_[info.unique_id()] = info;
  }

  emit DeviceAdded(info.unique_id());
}

void GioLister::MountChanged(GMount *mount) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByMount(mount);
    if (id.isNull())
      return;

    MountInfo new_info = ReadMountInfo(mount);

    // Ignore the change if the new info is useless
    if ((mounts_[id].filesystem_size != 0 && new_info.filesystem_size == 0) ||
        (!mounts_[id].filesystem_type.isEmpty() && new_info.filesystem_type.isEmpty())) {
      return;
    }
    mounts_[id] = new_info;
  }

  emit DeviceChanged(id);
}

void GioLister::MountRemoved(GMount *mount) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByMount(mount);
    if (id.isNull())
      return;

    mounts_.remove(id);
  }

  emit DeviceRemoved(id);
}

QString GioLister::ConvertAndFree(char *str) {
  QString ret = QString::fromUtf8(str);
  g_free(str);
  return ret;
}

GioLister::MountInfo GioLister::ReadMountInfo(GMount* mount) {
  MountInfo ret;

  // Get basic information
  ret.mount = mount;
  ret.name = ConvertAndFree(g_mount_get_name(mount));

  // Get the icon name(s)
  GIcon* icon = g_mount_get_icon(mount);
  if (G_IS_THEMED_ICON(icon)) {
    const char* const * icons = g_themed_icon_get_names(G_THEMED_ICON(icon));
    for (const char* const * p = icons ; *p ; ++p) {
      ret.icon_names << QString::fromUtf8(*p);
    }
  }
  g_object_unref(icon);

  GFile* root = g_mount_get_root(mount);

  // Get the mount path
  ret.mount_path = ConvertAndFree(g_file_get_path(root));
  ret.uri = ConvertAndFree(g_file_get_uri(root));

  // Query the filesystem info for size, free space, and type
  GError* error = NULL;
  GFileInfo* info = g_file_query_filesystem_info(root,
      G_FILE_ATTRIBUTE_FILESYSTEM_SIZE "," G_FILE_ATTRIBUTE_FILESYSTEM_FREE ","
      G_FILE_ATTRIBUTE_FILESYSTEM_TYPE, NULL, &error);
  if (error) {
    qWarning() << error->message;
    g_error_free(error);
  } else {
    ret.filesystem_size = g_file_info_get_attribute_uint64(
        info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
    ret.filesystem_free = g_file_info_get_attribute_uint64(
        info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
    ret.filesystem_type = QString::fromUtf8(g_file_info_get_attribute_string(
        info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE));
    g_object_unref(info);
  }

  // Query the file's info for a filesystem ID
  // Only afc devices (that I know of) give reliably unique IDs
  if (ret.filesystem_type == "afc") {
    error = NULL;
    info = g_file_query_info(root, G_FILE_ATTRIBUTE_ID_FILESYSTEM,
                             G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (error) {
      qWarning() << error->message;
      g_error_free(error);
    } else {
      ret.uuid = QString::fromUtf8(g_file_info_get_attribute_string(
          info, G_FILE_ATTRIBUTE_ID_FILESYSTEM));
      g_object_unref(info);
    }
  }

  g_object_unref(root);

  // Get information about the volume
  GVolume* volume = g_mount_get_volume(mount);
  if (volume) {
    ret.unix_device = ConvertAndFree(g_volume_get_identifier(
        volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE));
    g_object_unref(volume);
  }

  return ret;
}

QString GioLister::FindUniqueIdByMount(GMount *mount) const {
  foreach (const MountInfo& info, mounts_) {
    if (info.mount == mount)
      return info.unique_id();
  }
  return QString();
}

template <typename T, typename F>
void OperationFinished(F f, GObject *object, GAsyncResult *result) {
  T* obj = reinterpret_cast<T*>(object);
  GError* error = NULL;

  f(obj, result, &error);

  if (error) {
    qDebug() << "Unmount error:" << error->message;
    g_error_free(error);
  }
}

void GioLister::VolumeEjectFinished(GObject *object, GAsyncResult *result, gpointer) {
  OperationFinished<GVolume>(boost::bind(
      g_volume_eject_with_operation_finish, _1, _2, _3), object, result);
}

void GioLister::MountEjectFinished(GObject *object, GAsyncResult *result, gpointer) {
  OperationFinished<GMount>(boost::bind(
      g_mount_eject_with_operation_finish, _1, _2, _3), object, result);
}

void GioLister::MountUnmountFinished(GObject *object, GAsyncResult *result, gpointer) {
  OperationFinished<GMount>(boost::bind(
      g_mount_unmount_with_operation_finish, _1, _2, _3), object, result);
}

void GioLister::UnmountDevice(const QString &id) {
  GMount* mount = LockAndGetMountInfo(id, &MountInfo::mount);
  if (!mount)
    return;

  GVolume* volume = g_mount_get_volume(mount);
  if (volume) {
    if (g_volume_can_eject(volume)) {
      g_volume_eject(volume, G_MOUNT_UNMOUNT_NONE, NULL,
                     (GAsyncReadyCallback) VolumeEjectFinished, NULL);
      g_object_unref(volume);
      return;
    }
    g_object_unref(volume);
  }

  if (g_mount_can_eject(mount)) {
    g_mount_eject(mount, G_MOUNT_UNMOUNT_NONE, NULL,
                  (GAsyncReadyCallback) MountEjectFinished, NULL);
  } else if (g_mount_can_unmount(mount)) {
    g_mount_unmount(mount, G_MOUNT_UNMOUNT_NONE, NULL,
                    (GAsyncReadyCallback) MountUnmountFinished, NULL);
  }
}
