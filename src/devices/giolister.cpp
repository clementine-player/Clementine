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

#include <functional>
#include <memory>

#include <QFile>
#include <QStringList>
#include <QtDebug>

#include "giolister.h"
#include "core/logging.h"
#include "core/signalchecker.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

QString GioLister::DeviceInfo::unique_id() const {
  if (mount)
    return QString("Gio/%1/%2/%3").arg(mount_uuid, filesystem_type).arg(
        filesystem_size);

  return QString("Gio/unmounted/%1").arg((qulonglong)volume.get());
}

bool GioLister::DeviceInfo::is_suitable() const {
  if (!volume) return false;  // This excludes smb or ssh mounts

  if (drive && !drive_removable) return false;  // This excludes internal drives

  if (filesystem_type.isEmpty()) return true;

  return filesystem_type != "udf" && filesystem_type != "smb" &&
         filesystem_type != "cifs" && filesystem_type != "ssh" &&
         filesystem_type != "isofs";
}

template <typename T, typename F>
void OperationFinished(F f, GObject* object, GAsyncResult* result) {
  T* obj = reinterpret_cast<T*>(object);
  GError* error = nullptr;

  f(obj, result, &error);

  if (error) {
    qLog(Error) << "Mount/unmount error:"
                << QString::fromLocal8Bit(error->message);
    g_error_free(error);
  }
}

void GioLister::VolumeMountFinished(GObject* object, GAsyncResult* result,
                                    gpointer) {
  OperationFinished<GVolume>(std::bind(g_volume_mount_finish, _1, _2, _3),
                             object, result);
}

void GioLister::Init() {
  monitor_.reset_without_add(g_volume_monitor_get());

  // Get existing volumes
  GList* const volumes = g_volume_monitor_get_volumes(monitor_);
  for (GList* p = volumes; p; p = p->next) {
    GVolume* volume = static_cast<GVolume*>(p->data);

    VolumeAdded(volume);
    g_object_unref(volume);
  }
  g_list_free(volumes);

  // Get existing mounts
  GList* const mounts = g_volume_monitor_get_mounts(monitor_);
  for (GList* p = mounts; p; p = p->next) {
    GMount* mount = static_cast<GMount*>(p->data);

    MountAdded(mount);
    g_object_unref(mount);
  }
  g_list_free(mounts);

  // Connect signals from the monitor
  signals_.append(CHECKED_GCONNECT(monitor_, "volume-added", &VolumeAddedCallback, this));
  signals_.append(CHECKED_GCONNECT(monitor_, "volume-removed", &VolumeRemovedCallback, this));
  signals_.append(CHECKED_GCONNECT(monitor_, "mount-added", &MountAddedCallback, this));
  signals_.append(CHECKED_GCONNECT(monitor_, "mount-changed", &MountChangedCallback, this));
  signals_.append(CHECKED_GCONNECT(monitor_, "mount-removed", &MountRemovedCallback, this));
}

GioLister::~GioLister() {
  for (gulong signal : signals_) {
    g_signal_handler_disconnect(monitor_, signal);
  }
}

QStringList GioLister::DeviceUniqueIDs() {
  QMutexLocker l(&mutex_);
  return devices_.keys();
}

QVariantList GioLister::DeviceIcons(const QString& id) {
  QVariantList ret;
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id)) return ret;

  const DeviceInfo& info = devices_[id];

  if (info.mount) {
    ret << DeviceLister::GuessIconForPath(info.mount_path);
    ret << info.mount_icon_names;
  }

  ret << DeviceLister::GuessIconForModel(QString(), info.mount_name);

  return ret;
}

QString GioLister::DeviceManufacturer(const QString& id) { return QString(); }

QString GioLister::DeviceModel(const QString& id) {
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id)) return QString();
  const DeviceInfo& info = devices_[id];

  return info.drive_name.isEmpty() ? info.volume_name : info.drive_name;
}

quint64 GioLister::DeviceCapacity(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::filesystem_size);
}

quint64 GioLister::DeviceFreeSpace(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::filesystem_free);
}

QString GioLister::MakeFriendlyName(const QString& id) {
  return DeviceModel(id);
}

QVariantMap GioLister::DeviceHardwareInfo(const QString& id) {
  QVariantMap ret;

  QMutexLocker l(&mutex_);
  if (!devices_.contains(id)) return ret;
  const DeviceInfo& info = devices_[id];

  ret[QT_TR_NOOP("Mount point")] = info.mount_path;
  ret[QT_TR_NOOP("Device")] = info.volume_unix_device;
  ret[QT_TR_NOOP("URI")] = info.mount_uri;
  return ret;
}

QList<QUrl> GioLister::MakeDeviceUrls(const QString& id) {
  QString mount_point;
  QString uri;
  QString unix_device;
  {
    QMutexLocker l(&mutex_);
    mount_point = devices_[id].mount_path;
    uri = devices_[id].mount_uri;
    unix_device = devices_[id].volume_unix_device;
  }

  // gphoto2 gives invalid hostnames with []:, characters in
  uri.replace(QRegExp("//\\[usb:(\\d+),(\\d+)\\]"), "//usb-\\1-\\2");

  QUrl url(uri);

  QList<QUrl> ret;

  if (url.isValid()) {
    QRegExp device_re("usb/(\\d+)/(\\d+)");
    if (device_re.indexIn(unix_device) >= 0) {
      url.addQueryItem("busnum", device_re.cap(1));
      url.addQueryItem("devnum", device_re.cap(2));
    }

    // Special case for file:// GIO URIs - we have to check whether they point
    // to an ipod.
    if (url.scheme() == "file") {
      ret << MakeUrlFromLocalPath(url.path());
    } else {
      ret << url;
    }
  }

  ret << MakeUrlFromLocalPath(mount_point);
  return ret;
}

void GioLister::VolumeAddedCallback(GVolumeMonitor*, GVolume* v, gpointer d) {
  static_cast<GioLister*>(d)->VolumeAdded(v);
}

void GioLister::VolumeRemovedCallback(GVolumeMonitor*, GVolume* v, gpointer d) {
  static_cast<GioLister*>(d)->VolumeRemoved(v);
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

void GioLister::VolumeAdded(GVolume* volume) {
  g_object_ref(volume);

  DeviceInfo info;
  info.ReadVolumeInfo(volume);
#ifdef HAVE_AUDIOCD
  if (info.volume_root_uri.startsWith("cdda"))
    // Audio CD devices are already handled by CDDA lister
    return;
#endif
  info.ReadDriveInfo(g_volume_get_drive(volume));
  info.ReadMountInfo(g_volume_get_mount(volume));
  if (!info.is_suitable()) return;

  {
    QMutexLocker l(&mutex_);
    devices_[info.unique_id()] = info;
  }

  emit DeviceAdded(info.unique_id());
}

void GioLister::VolumeRemoved(GVolume* volume) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByVolume(volume);
    if (id.isNull()) return;

    devices_.remove(id);
  }

  emit DeviceRemoved(id);
}

void GioLister::MountAdded(GMount* mount) {
  g_object_ref(mount);

  DeviceInfo info;
  info.ReadVolumeInfo(g_mount_get_volume(mount));
#ifdef HAVE_AUDIOCD
  if (info.volume_root_uri.startsWith("cdda"))
    // Audio CD devices are already handled by CDDA lister
    return;
#endif
  info.ReadMountInfo(mount);
  info.ReadDriveInfo(g_mount_get_drive(mount));
  if (!info.is_suitable()) return;

  QString old_id;
  {
    QMutexLocker l(&mutex_);

    // The volume might already exist - either mounted or unmounted.
    for (const QString& id : devices_.keys()) {
      if (devices_[id].volume == info.volume) {
        old_id = id;
        break;
      }
    }

    if (!old_id.isEmpty() && old_id != info.unique_id()) {
      // If the ID has changed (for example, after it's been mounted), we need
      // to remove the old device.
      devices_.remove(old_id);
      emit DeviceRemoved(old_id);

      old_id = QString();
    }
    devices_[info.unique_id()] = info;
  }

  if (!old_id.isEmpty())
    emit DeviceChanged(old_id);
  else {
    emit DeviceAdded(info.unique_id());
  }
}

void GioLister::MountChanged(GMount* mount) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByMount(mount);
    if (id.isNull()) return;

    g_object_ref(mount);

    DeviceInfo new_info;
    new_info.ReadMountInfo(mount);
    new_info.ReadVolumeInfo(g_mount_get_volume(mount));
    new_info.ReadDriveInfo(g_mount_get_drive(mount));

    // Ignore the change if the new info is useless
    if (new_info.invalid_enclosing_mount ||
        (devices_[id].filesystem_size != 0 && new_info.filesystem_size == 0) ||
        (!devices_[id].filesystem_type.isEmpty() &&
         new_info.filesystem_type.isEmpty()))
      return;

    devices_[id] = new_info;
  }

  emit DeviceChanged(id);
}

void GioLister::MountRemoved(GMount* mount) {
  QString id;
  {
    QMutexLocker l(&mutex_);
    id = FindUniqueIdByMount(mount);
    if (id.isNull()) return;

    devices_.remove(id);
  }

  emit DeviceRemoved(id);
}

QString GioLister::DeviceInfo::ConvertAndFree(char* str) {
  QString ret = QString::fromUtf8(str);
  g_free(str);
  return ret;
}

void GioLister::DeviceInfo::ReadMountInfo(GMount* mount) {
  // Get basic information
  this->mount.reset_without_add(mount);
  if (!mount) return;

  mount_name = ConvertAndFree(g_mount_get_name(mount));

  // Get the icon name(s)
  mount_icon_names.clear();
  GIcon* icon = g_mount_get_icon(mount);
  if (G_IS_THEMED_ICON(icon)) {
    const char* const* icons = g_themed_icon_get_names(G_THEMED_ICON(icon));
    for (const char* const* p = icons; *p; ++p) {
      mount_icon_names << QString::fromUtf8(*p);
    }
  }
  g_object_unref(icon);

  GFile* root = g_mount_get_root(mount);

  // Get the mount path
  mount_path = ConvertAndFree(g_file_get_path(root));
  mount_uri = ConvertAndFree(g_file_get_uri(root));

  // Do a sanity check to make sure the root is actually this mount - when a
  // device is unmounted GIO sends a changed signal before the removed signal,
  // and we end up reading information about the / filesystem by mistake.
  GError* error = nullptr;
  GMount* actual_mount = g_file_find_enclosing_mount(root, nullptr, &error);
  if (error || !actual_mount) {
    g_error_free(error);
    invalid_enclosing_mount = true;
  } else if (actual_mount) {
    g_object_unref(actual_mount);
  }

  // Query the filesystem info for size, free space, and type
  error = nullptr;
  GFileInfo* info = g_file_query_filesystem_info(
      root, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE
      "," G_FILE_ATTRIBUTE_FILESYSTEM_FREE "," G_FILE_ATTRIBUTE_FILESYSTEM_TYPE,
      nullptr, &error);
  if (error) {
    qLog(Warning) << QString::fromLocal8Bit(error->message);
    g_error_free(error);
  } else {
    filesystem_size = g_file_info_get_attribute_uint64(
        info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
    filesystem_free = g_file_info_get_attribute_uint64(
        info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
    filesystem_type = QString::fromUtf8(g_file_info_get_attribute_string(
        info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE));
    g_object_unref(info);
  }

  // Query the file's info for a filesystem ID
  // Only afc devices (that I know of) give reliably unique IDs
  if (filesystem_type == "afc") {
    error = nullptr;
    info = g_file_query_info(root, G_FILE_ATTRIBUTE_ID_FILESYSTEM,
                             G_FILE_QUERY_INFO_NONE, nullptr, &error);
    if (error) {
      qLog(Warning) << QString::fromLocal8Bit(error->message);
      g_error_free(error);
    } else {
      mount_uuid = QString::fromUtf8(g_file_info_get_attribute_string(
          info, G_FILE_ATTRIBUTE_ID_FILESYSTEM));
      g_object_unref(info);
    }
  }

  g_object_unref(root);
}

void GioLister::DeviceInfo::ReadVolumeInfo(GVolume* volume) {
  this->volume.reset_without_add(volume);
  if (!volume) return;

  volume_name = ConvertAndFree(g_volume_get_name(volume));
  volume_uuid = ConvertAndFree(g_volume_get_uuid(volume));
  volume_unix_device = ConvertAndFree(
      g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE));

  GFile* root = g_volume_get_activation_root(volume);
  if (root) {
    volume_root_uri = g_file_get_uri(root);
    g_object_unref(root);
  }
}

void GioLister::DeviceInfo::ReadDriveInfo(GDrive* drive) {
  this->drive.reset_without_add(drive);
  if (!drive) return;

  drive_name = ConvertAndFree(g_drive_get_name(drive));
  drive_removable = g_drive_is_media_removable(drive);
}

QString GioLister::FindUniqueIdByMount(GMount* mount) const {
  for (const DeviceInfo& info : devices_) {
    if (info.mount == mount) return info.unique_id();
  }
  return QString();
}

QString GioLister::FindUniqueIdByVolume(GVolume* volume) const {
  for (const DeviceInfo& info : devices_) {
    if (info.volume == volume) return info.unique_id();
  }
  return QString();
}

void GioLister::VolumeEjectFinished(GObject* object, GAsyncResult* result,
                                    gpointer) {
  OperationFinished<GVolume>(
      std::bind(g_volume_eject_with_operation_finish, _1, _2, _3), object,
      result);
}

void GioLister::MountEjectFinished(GObject* object, GAsyncResult* result,
                                   gpointer) {
  OperationFinished<GMount>(
      std::bind(g_mount_eject_with_operation_finish, _1, _2, _3), object,
      result);
}

void GioLister::MountUnmountFinished(GObject* object, GAsyncResult* result,
                                     gpointer) {
  OperationFinished<GMount>(
      std::bind(g_mount_unmount_with_operation_finish, _1, _2, _3), object,
      result);
}

void GioLister::UnmountDevice(const QString& id) {
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id)) return;

  const DeviceInfo& info = devices_[id];

  if (!info.mount) return;

  if (info.volume) {
    if (g_volume_can_eject(info.volume)) {
      g_volume_eject_with_operation(
          info.volume, G_MOUNT_UNMOUNT_NONE, nullptr, nullptr,
          (GAsyncReadyCallback)VolumeEjectFinished, nullptr);
      g_object_unref(info.volume);
      return;
    }
  }

  if (g_mount_can_eject(info.mount)) {
    g_mount_eject_with_operation(
        info.mount, G_MOUNT_UNMOUNT_NONE, nullptr, nullptr,
        (GAsyncReadyCallback)MountEjectFinished, nullptr);
  } else if (g_mount_can_unmount(info.mount)) {
    g_mount_unmount_with_operation(
        info.mount, G_MOUNT_UNMOUNT_NONE, nullptr, nullptr,
        (GAsyncReadyCallback)MountUnmountFinished, nullptr);
  }
}

void GioLister::UpdateDeviceFreeSpace(const QString& id) {
  {
    QMutexLocker l(&mutex_);
    if (!devices_.contains(id)) return;

    DeviceInfo& device_info = devices_[id];

    GFile* root = g_mount_get_root(device_info.mount);

    GError* error = nullptr;
    GFileInfo* info = g_file_query_filesystem_info(
        root, G_FILE_ATTRIBUTE_FILESYSTEM_FREE, nullptr, &error);
    if (error) {
      qLog(Warning) << QString::fromLocal8Bit(error->message);
      g_error_free(error);
    } else {
      device_info.filesystem_free = g_file_info_get_attribute_uint64(
          info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
      g_object_unref(info);
    }

    g_object_unref(root);
  }

  emit DeviceChanged(id);
}

bool GioLister::DeviceNeedsMount(const QString& id) {
  QMutexLocker l(&mutex_);
  return devices_.contains(id) && !devices_[id].mount;
}

int GioLister::MountDevice(const QString& id) {
  const int request_id = next_mount_request_id_++;
  metaObject()->invokeMethod(this, "DoMountDevice", Qt::QueuedConnection,
                             Q_ARG(QString, id), Q_ARG(int, request_id));
  return request_id;
}

void GioLister::DoMountDevice(const QString& id, int request_id) {
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id)) {
    emit DeviceMounted(id, request_id, false);
    return;
  }

  const DeviceInfo& info = devices_[id];
  if (info.mount) {
    // Already mounted
    emit DeviceMounted(id, request_id, true);
    return;
  }

  g_volume_mount(info.volume, G_MOUNT_MOUNT_NONE, nullptr, nullptr,
                 VolumeMountFinished, nullptr);
  emit DeviceMounted(id, request_id, true);
}
