#include "udisks2lister.h"

#include <QDBusConnection>

#include "core/logging.h"
#include "core/utilities.h"

#include "dbus/udisks2filesystem.h"
#include "dbus/udisks2block.h"
#include "dbus/udisks2drive.h"
#include "dbus/udisks2job.h"

const QString Udisks2Lister::udisks2service_ = "org.freedesktop.UDisks2";

Udisks2Lister::Udisks2Lister() {

}

Udisks2Lister::~Udisks2Lister() {
  qLog(Debug) << __PRETTY_FUNCTION__;
}

QStringList Udisks2Lister::DeviceUniqueIDs() {
  QReadLocker locker(&device_data_lock_);
  return device_data_.keys();
}

QVariantList Udisks2Lister::DeviceIcons(const QString &id) {
  return QVariantList();
}

QString Udisks2Lister::DeviceManufacturer(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return "";
  return device_data_[id].vendor;
}

QString Udisks2Lister::DeviceModel(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return "";
  return device_data_[id].model;
}

quint64 Udisks2Lister::DeviceCapacity(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return 0;
  return device_data_[id].capacity;
}

quint64 Udisks2Lister::DeviceFreeSpace(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return 0;
  return device_data_[id].free_space;
}

QVariantMap Udisks2Lister::DeviceHardwareInfo(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return QVariantMap();

  QVariantMap result;

  const auto &data = device_data_[id];
  result[QT_TR_NOOP("DBus path")] = data.dbus_path;
  result[QT_TR_NOOP("Serial number")] = data.serial;
  result[QT_TR_NOOP("Mount points")] = data.mount_paths.join(", ");
  result[QT_TR_NOOP("Parition label")] = data.label;
  result[QT_TR_NOOP("UUID")] = data.uuid;

  return result;
}

QString Udisks2Lister::MakeFriendlyName(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return "";
  return device_data_[id].friendly_name;
}

QList<QUrl> Udisks2Lister::MakeDeviceUrls(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return QList<QUrl>();
  return QList<QUrl>() << QUrl::fromLocalFile(device_data_[id].mount_paths.at(0));
}

void Udisks2Lister::UnmountDevice(const QString &id) {
  QReadLocker locker(&device_data_lock_);
  if (!device_data_.contains(id))
    return;

  OrgFreedesktopUDisks2FilesystemInterface filesystem(
        udisks2service_,
        device_data_[id].dbus_path,
        QDBusConnection::systemBus());

  if (filesystem.isValid())
  {
    auto umountResult = filesystem.Unmount(QVariantMap());
    umountResult.waitForFinished();

    OrgFreedesktopUDisks2DriveInterface drive(
          udisks2service_,
          device_data_[id].dbus_drive_path,
          QDBusConnection::systemBus());

    if (drive.isValid())
    {
      auto ejectResult = drive.Eject(QVariantMap());
      ejectResult.waitForFinished();
    }
  }
}

void Udisks2Lister::UpdateDeviceFreeSpace(const QString &id) {
  QWriteLocker locker(&device_data_lock_);
  device_data_[id].free_space = Utilities::FileSystemFreeSpace(device_data_[id].mount_paths.at(0));

  emit DeviceChanged(id);
}

void Udisks2Lister::Init() {
  udisks2_interface_.reset(new OrgFreedesktopDBusObjectManagerInterface(
                             udisks2service_,
                             "/org/freedesktop/UDisks2",
                             QDBusConnection::systemBus()));

  QDBusPendingReply<ManagedObjectList> reply = udisks2_interface_->GetManagedObjects();
  reply.waitForFinished();

  if (!reply.isValid()) {
    qLog(Warning) << "Error enumerating udisks2 devices:"
                  << reply.error().name() << reply.error().message();
    udisks2_interface_.reset();
    return;
  }

  for (const QDBusObjectPath &path : reply.value().keys()) {
    auto partitionData = ReadPartitionData(path, false);

    if (!partitionData.dbus_path.isEmpty())
    {
      QWriteLocker locker(&device_data_lock_);
      device_data_[partitionData.unique_id()] = partitionData;
    }
  }

  for (const auto &id : device_data_.keys()) {
    emit DeviceAdded(id);
  }

  connect(udisks2_interface_.get(), SIGNAL(InterfacesAdded(QDBusObjectPath, InterfacesAndProperties)),
          SLOT(DBusInterfaceAdded(QDBusObjectPath, InterfacesAndProperties)));
  connect(udisks2_interface_.get(), SIGNAL(InterfacesRemoved(QDBusObjectPath, QStringList)),
          SLOT(DBusInterfaceRemoved(QDBusObjectPath, QStringList)));
}

void Udisks2Lister::DBusInterfaceAdded(const QDBusObjectPath &path,
                                       const InterfacesAndProperties &interfaces) {
  for (auto interface = interfaces.constBegin(); interface != interfaces.constEnd(); ++interface)
  {
    if (interface.key() != "org.freedesktop.UDisks2.Job")
      continue;

    std::shared_ptr<OrgFreedesktopUDisks2JobInterface> job = std::make_shared<OrgFreedesktopUDisks2JobInterface>(
          udisks2service_,
          path.path(),
          QDBusConnection::systemBus());

    if (!job->isValid())
      continue;

    // For some reason access through Job interface returns all properties
    // as invalid for unmount job so we're doing this the hard way
    bool isMountJob = false;
    if (interface.value()["Operation"] == "filesystem-mount")
      isMountJob = true;
    else if (interface.value()["Operation"] == "filesystem-unmount")
      isMountJob = false;
    else
      continue;

    QList<QDBusObjectPath> mountedParititons
        = GetMountedPartitionsFromDBusArgument(interface.value()["Objects"].value<QDBusArgument>());

    if (mountedParititons.isEmpty()) {
      qLog(Warning) << "Empty Udisks2 mount/umount job " << path.path();
      continue;
    }

    {
      QMutexLocker locker(&jobs_lock_);
      qLog(Debug) << "Adding pending job | DBus Path = " << job->path()
                  << " | IsMountJob = " << isMountJob
                  << " | First partition = " << mountedParititons.at(0).path();
      mounting_jobs_[path].dbus_interface = job;
      mounting_jobs_[path].isMount = isMountJob;
      mounting_jobs_[path].mounted_partitions = mountedParititons;
      connect(job.get(), SIGNAL(Completed(bool, const QString&)),
              SLOT(JobCompleted(bool, const QString&)));
    }
  }
}

void Udisks2Lister::DBusInterfaceRemoved(const QDBusObjectPath &path, const QStringList &ifaces) {
  if (!isPendingJob(path))
    RemoveDevice(path);
}

bool Udisks2Lister::isPendingJob(const QDBusObjectPath &jobPath) {
  QMutexLocker locker(&jobs_lock_);

  if (!mounting_jobs_.contains(jobPath))
    return false;

  mounting_jobs_.remove(jobPath);
  return true;
}

void Udisks2Lister::RemoveDevice(const QDBusObjectPath &devicePath) {
  QWriteLocker locker(&device_data_lock_);
  QString id;
  for (const auto &data : device_data_) {
    if (data.dbus_path == devicePath.path()) {
      id = data.unique_id();
      break;
    }
  }

  if (id.isEmpty())
    return;

  qLog(Debug) << "UDisks2 device removed: " << devicePath.path();
  device_data_.remove(id);
  DeviceRemoved(id);
}

QList<QDBusObjectPath> Udisks2Lister::GetMountedPartitionsFromDBusArgument(const QDBusArgument &input) {
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

void Udisks2Lister::JobCompleted(bool success, const QString &message) {
  auto job = qobject_cast<OrgFreedesktopUDisks2JobInterface*>(sender());
  QDBusObjectPath jobPath(job->path());

  if (!job->isValid()
      || !success
      || !mounting_jobs_.contains(jobPath))
    return;

  qLog(Debug) << "Pending Job Completed | Path = " << job->path()
              << " | Mount? = " << mounting_jobs_[jobPath].isMount
              << " | Success = " << success;

  for (const auto &mountedObject : mounting_jobs_[jobPath].mounted_partitions) {
    auto data = ReadPartitionData(mountedObject, false);
    if (data.dbus_path.isEmpty())
      continue;

    if (mounting_jobs_[jobPath].isMount) {
      qLog(Debug) << "UDisks2 mount job finished: Drive = " << data.dbus_drive_path
                  << " | Partition = " << data.dbus_path;
      QWriteLocker locker(&device_data_lock_);
      device_data_[data.unique_id()] = data;
      DeviceAdded(data.unique_id());
    } else {
      QWriteLocker locker(&device_data_lock_);
      QString id;
      for (auto &data : device_data_) {
        if (data.mount_paths.contains(mountedObject.path())) {
          qLog(Debug) << "UDisks2 umount job finished, found corresponding device: Drive = " << data.dbus_drive_path
                      << " | Partition = " << data.dbus_path;
          data.mount_paths.removeOne(mountedObject.path());
          if (data.mount_paths.empty())
            id = data.unique_id();
          break;
        }
      }

      if (!id.isEmpty()) {
        qLog(Debug) << "Partition " << data.dbus_path << " has no more mount points, removing it from device list";
        device_data_.remove(id);
        DeviceRemoved(id);
      }
    }
  }
}

Udisks2Lister::PartitionData Udisks2Lister::ReadPartitionData(const QDBusObjectPath &path,
                                                              bool beingMounted) {
  PartitionData result;
  OrgFreedesktopUDisks2FilesystemInterface filesystem(
        udisks2service_,
        path.path(),
        QDBusConnection::systemBus());
  OrgFreedesktopUDisks2BlockInterface block(
        udisks2service_,
        path.path(),
        QDBusConnection::systemBus());

  if (filesystem.isValid()
      && block.isValid()
      && (beingMounted || !filesystem.mountPoints().empty())) {

    OrgFreedesktopUDisks2DriveInterface drive(
          udisks2service_,
          block.drive().path(),
          QDBusConnection::systemBus());

    if (drive.isValid()
        && drive.mediaRemovable()) {
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

      for (const auto &path : filesystem.mountPoints())
        result.mount_paths.push_back(path);

      result.free_space = Utilities::FileSystemFreeSpace(result.mount_paths.at(0));
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
