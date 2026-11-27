/* This file is part of Clementine.
   Copyright 2016, Valeriy Malov <jazzvoid@gmail.com>

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

#include "udisks2lister.h"

#include <QDBusConnection>
#include <QTimer>

#include "core/logging.h"
#include "core/utilities.h"
#include "dbus/objectmanager.h"
#include "dbus/udisks2block.h"
#include "dbus/udisks2drive.h"
#include "dbus/udisks2filesystem.h"
#include "dbus/udisks2job.h"

constexpr char Udisks2Lister::udisks2_service_[];

Udisks2Lister::Udisks2Lister() {}

Udisks2Lister::~Udisks2Lister() {}

QStringList Udisks2Lister::DeviceUniqueIDs() {
  QReadLocker locker(&device_data_lock_);
  return device_data_.keys();
}

QVariantList Udisks2Lister::DeviceIcons(const QString& id) {
  // Copy vendor/model under one QReadLocker instead of calling
  // DeviceManufacturer()/DeviceModel(), which would recursively re-lock a
  // non-recursive QReadWriteLock and deadlock against a pending writer.
  QString path;
  QString vendor;
  QString model;
  {
    QReadLocker locker(&device_data_lock_);
    if (!device_data_.contains(id)) return QVariantList();
    const PartitionData& data = device_data_[id];
    if (!data.mount_paths.isEmpty()) {
      path = data.mount_paths.at(0);
    }
    vendor = data.vendor;
    model = data.model;
  }
  QVariantList icons;
  if (!path.isEmpty()) {
    icons << GuessIconForPath(path);
  }
  icons << GuessIconForModel(vendor, model);
  return icons;
}

QString Udisks2Lister::DeviceManufacturer(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id)) return "";
  return device_data_[id].vendor;
}

QString Udisks2Lister::DeviceModel(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id)) return "";
  return device_data_[id].model;
}

quint64 Udisks2Lister::DeviceCapacity(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id)) return 0;
  return device_data_[id].capacity;
}

quint64 Udisks2Lister::DeviceFreeSpace(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id)) return 0;
  return device_data_[id].free_space;
}

QVariantMap Udisks2Lister::DeviceHardwareInfo(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id)) return QVariantMap();

  QVariantMap result;

  const auto& data = device_data_[id];
  result[QT_TR_NOOP("D-Bus path")] = data.dbus_path;
  result[QT_TR_NOOP("Serial number")] = data.serial;
  result[QT_TR_NOOP("Mount points")] = data.mount_paths.join(", ");
  result[QT_TR_NOOP("Partition label")] = data.label;
  result[QT_TR_NOOP("UUID")] = data.uuid;

  return result;
}

QString Udisks2Lister::MakeFriendlyName(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id)) return "";
  return device_data_[id].friendly_name;
}

QList<QUrl> Udisks2Lister::MakeDeviceUrls(const QString& id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id) || device_data_[id].mount_paths.isEmpty()) {
    return QList<QUrl>();
  }
  return QList<QUrl>() << MakeUrlFromLocalPath(
             device_data_[id].mount_paths.at(0));
}

void Udisks2Lister::UnmountDevice(const QString& id) {
  // Copy D-Bus paths under QReadLocker, run blocking D-Bus unmount/eject calls
  // unlocked, then acquire QWriteLocker to remove the entry before emitting.
  QString dbus_path;
  QString dbus_drive_path;
  {
    QReadLocker locker(&device_data_lock_);
    if (!device_data_.contains(id)) return;
    dbus_path = device_data_[id].dbus_path;
    dbus_drive_path = device_data_[id].dbus_drive_path;
  }

  OrgFreedesktopUDisks2FilesystemInterface filesystem(
      udisks2_service_, dbus_path, QDBusConnection::systemBus());

  if (filesystem.isValid()) {
    auto unmount_result = filesystem.Unmount(QVariantMap());
    unmount_result.waitForFinished();

    if (unmount_result.isError()) {
      qLog(Warning) << "Failed to unmount " << id << ": "
                    << unmount_result.error();
      return;
    }

    OrgFreedesktopUDisks2DriveInterface drive(udisks2_service_, dbus_drive_path,
                                              QDBusConnection::systemBus());

    if (drive.isValid()) {
      auto eject_result = drive.Eject(QVariantMap());
      eject_result.waitForFinished();

      if (eject_result.isError())
        qLog(Warning) << "Failed to eject " << id << ": "
                      << eject_result.error();
    }

    {
      QWriteLocker locker(&device_data_lock_);
      device_data_.remove(id);
    }
    emit DeviceRemoved(id);
  }
}

void Udisks2Lister::UpdateDeviceFreeSpace(const QString& id) {
  // Stat the filesystem outside the lock, then re-verify id before writing.
  QString mount_path;
  {
    QReadLocker locker(&device_data_lock_);
    if (!device_data_.contains(id) || device_data_[id].mount_paths.isEmpty()) {
      return;
    }
    mount_path = device_data_[id].mount_paths.at(0);
  }

  const quint64 free_space = Utilities::FileSystemFreeSpace(mount_path);
  {
    QWriteLocker locker(&device_data_lock_);
    if (!device_data_.contains(id)) return;
    device_data_[id].free_space = free_space;
  }

  emit DeviceChanged(id);
}

void Udisks2Lister::Init() {
  udisks2_interface_.reset(new OrgFreedesktopDBusObjectManagerInterface(
      udisks2_service_, "/org/freedesktop/UDisks2",
      QDBusConnection::systemBus()));

  QDBusPendingReply<ManagedObjectList> reply =
      udisks2_interface_->GetManagedObjects();
  reply.waitForFinished();

  if (!reply.isValid()) {
    qLog(Warning) << "Error enumerating udisks2 devices:"
                  << reply.error().name() << reply.error().message();
    udisks2_interface_.reset();
    return;
  }

  for (const QDBusObjectPath& path : reply.value().keys()) {
    auto partition_data = ReadPartitionData(path);

    if (!partition_data.dbus_path.isEmpty()) {
      QWriteLocker locker(&device_data_lock_);
      device_data_[partition_data.unique_id()] = partition_data;
    }
  }

  // Snapshot IDs so DeviceAdded() is emitted without holding device_data_lock_.
  QStringList added_ids;
  {
    QReadLocker locker(&device_data_lock_);
    added_ids = device_data_.keys();
  }
  for (const auto& id : added_ids) {
    emit DeviceAdded(id);
  }

  connect(udisks2_interface_.get(),
          SIGNAL(InterfacesAdded(QDBusObjectPath, InterfacesAndProperties)),
          SLOT(DBusInterfaceAdded(QDBusObjectPath, InterfacesAndProperties)));
  connect(udisks2_interface_.get(),
          SIGNAL(InterfacesRemoved(QDBusObjectPath, QStringList)),
          SLOT(DBusInterfaceRemoved(QDBusObjectPath, QStringList)));
}

void Udisks2Lister::DBusInterfaceAdded(
    const QDBusObjectPath& path, const InterfacesAndProperties& interfaces) {
  for (auto interface = interfaces.constBegin();
       interface != interfaces.constEnd(); ++interface) {
    if (interface.key() != "org.freedesktop.UDisks2.Job") continue;

    std::shared_ptr<OrgFreedesktopUDisks2JobInterface> job =
        std::make_shared<OrgFreedesktopUDisks2JobInterface>(
            udisks2_service_, path.path(), QDBusConnection::systemBus());

    if (!job->isValid()) continue;

    bool is_mount_job = false;
    if (job->operation() == "filesystem-mount") {
      is_mount_job = true;
    } else if (job->operation() == "filesystem-unmount") {
      is_mount_job = false;
    } else {
      continue;
    }

    auto mounted_partitions = job->objects();

    if (mounted_partitions.isEmpty()) {
      qLog(Warning) << "Empty Udisks2 mount/umount job " << path.path();
      continue;
    }

    {
      QMutexLocker locker(&jobs_lock_);
      qLog(Debug) << "Adding pending job | DBus Path = " << job->path()
                  << " | IsMountJob = " << is_mount_job
                  << " | First partition = " << mounted_partitions.at(0).path();
      mounting_jobs_[path].dbus_interface = job;
      mounting_jobs_[path].is_mount = is_mount_job;
      mounting_jobs_[path].mounted_partitions = mounted_partitions;
      connect(job.get(), SIGNAL(Completed(bool, const QString&)),
              SLOT(JobCompleted(bool, const QString&)));
    }
  }
}

void Udisks2Lister::DBusInterfaceRemoved(const QDBusObjectPath& path,
                                         const QStringList& ifaces) {
  if (!isPendingJob(path)) RemoveDevice(path);
}

bool Udisks2Lister::isPendingJob(const QDBusObjectPath& job_path) {
  QMutexLocker locker(&jobs_lock_);

  if (!mounting_jobs_.contains(job_path)) return false;

  mounting_jobs_.remove(job_path);
  return true;
}

void Udisks2Lister::RemoveDevice(const QDBusObjectPath& device_path) {
  QString id;
  {
    QWriteLocker locker(&device_data_lock_);
    for (const auto& data : device_data_) {
      if (data.dbus_path == device_path.path()) {
        id = data.unique_id();
        break;
      }
    }

    if (id.isEmpty()) return;

    qLog(Debug) << "UDisks2 device removed: " << device_path.path();
    device_data_.remove(id);
  }
  // Emit after releasing device_data_lock_ so slots can query Udisks2Lister.
  emit DeviceRemoved(id);
}

QList<QDBusObjectPath> Udisks2Lister::GetMountedPartitionsFromDBusArgument(
    const QDBusArgument& input) {
  QList<QDBusObjectPath> result;

  input.beginArray();
  while (!input.atEnd()) {
    QDBusObjectPath extractedPath;
    input >> extractedPath;
    result.push_back(extractedPath);
  }
  input.endArray();

  return result;
}

void Udisks2Lister::JobCompleted(bool success, const QString& message) {
  auto job = qobject_cast<OrgFreedesktopUDisks2JobInterface*>(sender());
  if (!job) return;
  QDBusObjectPath jobPath(job->path());

  // Take the job under jobs_lock_ so D-Bus reads/removal run unlocked.
  Udisks2Job job_info;
  {
    QMutexLocker locker(&jobs_lock_);
    if (!job->isValid() || !mounting_jobs_.contains(jobPath)) {
      return;
    }
    job_info = mounting_jobs_.take(jobPath);
  }

  qLog(Debug) << "Pending Job Completed | Path = " << job->path()
              << " | Mount? = " << job_info.is_mount
              << " | Success = " << success;

  // Defer destroying the job interface (sender()) past this slot so Qt's
  // signal dispatch finishes cleanly even when success is false.
  if (job_info.dbus_interface) {
    auto held_job = std::move(job_info.dbus_interface);
    QTimer::singleShot(0, this, [held_job]() mutable { held_job.reset(); });
  }
  if (!success) return;

  for (const auto& mounted_object : job_info.mounted_partitions) {
    if (job_info.is_mount) {
      auto partition_data = ReadPartitionData(mounted_object);
      if (!partition_data.dbus_path.isEmpty()) {
        HandleFinishedMountJob(partition_data);
      }
    } else {
      HandleFinishedUnmountJob(mounted_object);
    }
  }
}

void Udisks2Lister::HandleFinishedMountJob(
    const Udisks2Lister::PartitionData& partition_data) {
  qLog(Debug) << "UDisks2 mount job finished: Drive = "
              << partition_data.dbus_drive_path
              << " | Partition = " << partition_data.dbus_path;
  // Release device_data_lock_ before emitting DeviceAdded().
  const QString id = partition_data.unique_id();
  {
    QWriteLocker locker(&device_data_lock_);
    device_data_[id] = partition_data;
  }
  emit DeviceAdded(id);
}

void Udisks2Lister::HandleFinishedUnmountJob(
    const QDBusObjectPath& mounted_object) {
  // Query remaining mount points unlocked; mounted_object is the partition's
  // UDisks2 D-Bus object path (matching data->dbus_path, not mount_paths).
  QStringList remaining_mounts;
  OrgFreedesktopUDisks2FilesystemInterface filesystem(
      udisks2_service_, mounted_object.path(), QDBusConnection::systemBus());
  if (filesystem.isValid()) {
    for (const auto& path : filesystem.mountPoints()) {
      remaining_mounts.push_back(path);
    }
  }

  QString removed_id;
  {
    QWriteLocker locker(&device_data_lock_);
    for (auto it = device_data_.begin(); it != device_data_.end(); ++it) {
      PartitionData* data = &it.value();
      if (data->dbus_path == mounted_object.path()) {
        qLog(Debug) << "UDisks2 umount job finished, found corresponding "
                       "device: Drive = "
                    << data->dbus_drive_path
                    << " | Partition = " << data->dbus_path;
        data->mount_paths = remaining_mounts;
        if (data->mount_paths.isEmpty()) {
          removed_id = data->unique_id();
        }
        break;
      }
    }

    if (!removed_id.isEmpty()) {
      qLog(Debug) << "Partition " << mounted_object.path()
                  << " has no more mount points, removing it from device list";
      device_data_.remove(removed_id);
    }
  }

  if (!removed_id.isEmpty()) {
    emit DeviceRemoved(removed_id);
  }
}

Udisks2Lister::PartitionData Udisks2Lister::ReadPartitionData(
    const QDBusObjectPath& path) {
  PartitionData result;
  OrgFreedesktopUDisks2FilesystemInterface filesystem(
      udisks2_service_, path.path(), QDBusConnection::systemBus());
  OrgFreedesktopUDisks2BlockInterface block(udisks2_service_, path.path(),
                                            QDBusConnection::systemBus());

  if (filesystem.isValid() && block.isValid() &&
      !filesystem.mountPoints().empty()) {
    OrgFreedesktopUDisks2DriveInterface drive(
        udisks2_service_, block.drive().path(), QDBusConnection::systemBus());

    if (drive.isValid() && drive.mediaRemovable()) {
      result.dbus_path = path.path();
      result.dbus_drive_path = block.drive().path();

      result.serial = drive.serial();
      result.vendor = drive.vendor();
      result.model = drive.model();

      result.label = block.idLabel();
      result.uuid = block.idUUID();
      result.capacity = drive.size();

      if (!result.label.isEmpty())
        result.friendly_name = result.label;
      else
        result.friendly_name = result.model + " " + result.uuid;

      for (const auto& path : filesystem.mountPoints())
        result.mount_paths.push_back(path);

      result.free_space =
          Utilities::FileSystemFreeSpace(result.mount_paths.at(0));
    }
  }

  return result;
}

QString Udisks2Lister::PartitionData::unique_id() const {
  return QString("Udisks2/%1/%2/%3/%4/%5")
      .arg(serial, vendor, model)
      .arg(capacity)
      .arg(uuid);
}

Udisks2Lister::Udisks2Job::Udisks2Job() : is_mount(true) {}

Udisks2Lister::PartitionData::PartitionData() : capacity(0), free_space(0) {}
