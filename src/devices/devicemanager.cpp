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
#include "devicedatabasebackend.h"
#include "devicemanager.h"
#include "devicekitlister.h"
#include "filesystemdevice.h"
#include "giolister.h"
#include "gpoddevice.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "ui/iconloader.h"

#include <QIcon>
#include <QPainter>
#include <QUrl>

const int DeviceManager::kDeviceIconSize = 32;
const int DeviceManager::kDeviceIconOverlaySize = 16;

DeviceManager::DeviceInfo::DeviceInfo()
  : database_id_(-1),
    lister_(NULL),
    task_percentage_(-1)
{
}

DeviceDatabaseBackend::Device DeviceManager::DeviceInfo::SaveToDb() const {
  DeviceDatabaseBackend::Device ret;
  ret.friendly_name_ = friendly_name_;
  ret.size_ = size_;
  ret.unique_id_ = unique_id_;
  ret.id_ = database_id_;

  if (lister_)
    ret.icon_name_ = lister_->DeviceIcon(unique_id_);

  return ret;
}

void DeviceManager::DeviceInfo::InitFromDb(const DeviceDatabaseBackend::Device &dev) {
  database_id_ = dev.id_;
  friendly_name_ = dev.friendly_name_;
  unique_id_ = dev.unique_id_;
  size_ = dev.size_;
  LoadIcon(dev.icon_name_);
}

void DeviceManager::DeviceInfo::LoadIcon(const QString &filename) {
  // Try to load the icon with that exact name first
  icon_name_ = filename;
  icon_ = IconLoader::Load(icon_name_);

  // If that failed than try to guess if it's a phone or ipod.  Fall back on
  // a usb memory stick icon.
  if (icon_.isNull()) {
    if (filename.contains("phone"))
      icon_name_ = "phone";
    else if (filename.contains("ipod") || filename.contains("apple"))
      icon_name_ = "multimedia-player-ipod-standard-monochrome";
    else
      icon_name_ = "drive-removable-media-usb-pendrive";
    icon_ = IconLoader::Load(icon_name_);
  }
}


DeviceManager::DeviceManager(BackgroundThread<Database>* database,
                             TaskManager* task_manager, QObject *parent)
  : QAbstractListModel(parent),
    database_(database),
    task_manager_(task_manager),
    not_connected_overlay_(IconLoader::Load("edit-delete"))
{
  connect(task_manager_, SIGNAL(TasksChanged()), SLOT(TasksChanged()));

  // Create the backend in the database thread
  backend_ = database_->CreateInThread<DeviceDatabaseBackend>();
  backend_->Init(database_->Worker());

  DeviceDatabaseBackend::DeviceList devices = backend_->GetAllDevices();
  foreach (const DeviceDatabaseBackend::Device& device, devices) {
    DeviceInfo info;
    info.InitFromDb(device);
    devices_ << info;
  }

#ifdef Q_WS_X11
  AddLister(new DeviceKitLister);
#endif
#ifdef HAVE_GIO
  AddLister(new GioLister);
#endif

  AddDeviceClass<FilesystemDevice>();

#ifdef HAVE_LIBGPOD
  AddDeviceClass<GPodDevice>();
#endif
}

DeviceManager::~DeviceManager() {
  qDeleteAll(listers_);
  backend_->deleteLater();
}

int DeviceManager::rowCount(const QModelIndex&) const {
  return devices_.count();
}

QVariant DeviceManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0)
    return QVariant();

  const DeviceInfo& info = devices_[index.row()];

  switch (role) {
    case Qt::DisplayRole: {
      QString text = info.friendly_name_.isEmpty() ? info.unique_id_ : info.friendly_name_;
      if (info.size_)
        text = text + QString(" (%1)").arg(Utilities::PrettySize(info.size_));
      return text;
    }

    case Qt::DecorationRole: {
      QPixmap pixmap = info.icon_.pixmap(kDeviceIconSize);

      if (!info.lister_) {
        // Disconnected but remembered
        QPainter p(&pixmap);
        p.drawPixmap(kDeviceIconSize - kDeviceIconOverlaySize,
                     kDeviceIconSize - kDeviceIconOverlaySize,
                     not_connected_overlay_.pixmap(kDeviceIconOverlaySize));
      }

      return pixmap;
    }

    case Role_FriendlyName:
      return info.friendly_name_;

    case Role_UniqueId:
      return info.unique_id_;

    case Role_IconName:
      return info.icon_name_;

    case Role_Capacity:
      return info.size_;

    case Role_State:
      if (info.device_)
        return State_Connected;
      if (info.lister_)
        return State_NotConnected;
      return State_Remembered;

    case Role_UpdatingPercentage:
      if (info.task_percentage_ == -1)
        return QVariant();
      return info.task_percentage_;

    default:
      return QVariant();
  }
}

void DeviceManager::AddLister(DeviceLister *lister) {
  listers_ << lister;
  connect(lister, SIGNAL(DeviceAdded(QString)), SLOT(PhysicalDeviceAdded(QString)));
  connect(lister, SIGNAL(DeviceRemoved(QString)), SLOT(PhysicalDeviceRemoved(QString)));
  connect(lister, SIGNAL(DeviceChanged(QString)), SLOT(PhysicalDeviceChanged(QString)));

  lister->Start();
}

int DeviceManager::FindDeviceById(const QString &id) const {
  for (int i=0 ; i<devices_.count() ; ++i) {
    if (devices_[i].unique_id_ == id)
      return i;
  }
  return -1;
}

void DeviceManager::PhysicalDeviceAdded(const QString &id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  qDebug() << "Device added:" << id;

  // Do we have this device already?
  int i = FindDeviceById(id);
  if (i == -1) {
    DeviceInfo info;
    info.lister_ = lister;
    info.unique_id_ = id;
    info.friendly_name_ = lister->MakeFriendlyName(id);
    info.size_ = lister->DeviceCapacity(id);
    info.LoadIcon(lister->DeviceIcon(id));

    beginInsertRows(QModelIndex(), devices_.count(), devices_.count());
    devices_ << info;
    endInsertRows();
  } else {
    DeviceInfo& info = devices_[i];

    info.lister_ = lister;
    emit dataChanged(index(i, 0), index(i, 0));
  }
}

void DeviceManager::PhysicalDeviceRemoved(const QString &id) {
  qDebug() << "Device removed:" << id;

  int i = FindDeviceById(id);
  if (i == -1) {
    // Shouldn't happen
    return;
  }

  DeviceInfo& info = devices_[i];

  if (info.database_id_ != -1) {
    // Keep the structure around, but just "disconnect" it
    info.lister_ = NULL;
    info.device_.reset();

    emit dataChanged(index(i, 0), index(i, 0));
    emit DeviceDisconnected(i);
  } else {
    // Remove the item from the model
    beginRemoveRows(QModelIndex(), i, i);
    devices_.removeAt(i);

    foreach (const QModelIndex& idx, persistentIndexList()) {
      if (idx.row() == i)
        changePersistentIndex(idx, QModelIndex());
      else if (idx.row() > i)
        changePersistentIndex(idx, index(idx.row()-1, idx.column()));
    }

    endRemoveRows();
  }
}

void DeviceManager::PhysicalDeviceChanged(const QString &id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());
  Q_UNUSED(lister);

  int i = FindDeviceById(id);
  if (i == -1) {
    // Shouldn't happen
    return;
  }

  // TODO
}

boost::shared_ptr<ConnectedDevice> DeviceManager::Connect(int row) {
  DeviceInfo& info = devices_[row];
  if (info.device_) // Already connected
    return info.device_;

  boost::shared_ptr<ConnectedDevice> ret;

  if (!info.lister_) // Not physically connected
    return ret;

  bool first_time = (info.database_id_ == -1);
  if (first_time) {
    // We haven't stored this device in the database before
    info.database_id_ = backend_->AddDevice(info.SaveToDb());
  }

  // Get the device URL
  QUrl url = info.lister_->MakeDeviceUrl(info.unique_id_);
  if (url.isEmpty())
    return ret;

  // Find a device class for this URL's scheme
  if (!device_classes_.contains(url.scheme())) {
    emit Error(tr("This type of device is not supported: %1").arg(url.toString()));
    return ret;
  }

  QMetaObject meta_object = device_classes_.value(url.scheme());
  QObject* instance = meta_object.newInstance(
      Q_ARG(QUrl, url), Q_ARG(DeviceLister*, info.lister_),
      Q_ARG(QString, info.unique_id_), Q_ARG(DeviceManager*, this),
      Q_ARG(int, info.database_id_), Q_ARG(bool, first_time));
  ret.reset(static_cast<ConnectedDevice*>(instance));

  if (!ret) {
    qWarning() << "Could not create device for" << url.toString();
  } else {
    info.device_ = ret;
    connect(info.device_.get(), SIGNAL(TaskStarted(int)), SLOT(DeviceTaskStarted(int)));
    connect(info.device_.get(), SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  }

  return ret;
}

boost::shared_ptr<ConnectedDevice> DeviceManager::GetConnectedDevice(int row) const {
  return devices_[row].device_;
}

int DeviceManager::GetDatabaseId(int row) const {
  return devices_[row].database_id_;
}

DeviceLister* DeviceManager::GetLister(int row) const {
  return devices_[row].lister_;
}

void DeviceManager::Disconnect(int row) {
  DeviceInfo& info = devices_[row];
  if (!info.device_) // Already disconnected
    return;

  info.device_.reset();
  emit DeviceDisconnected(row);
}

void DeviceManager::Forget(int row) {
  DeviceInfo& info = devices_[row];
  if (info.database_id_ == -1)
    return;

  if (info.device_)
    Disconnect(row);

  backend_->RemoveDevice(info.database_id_);
  info.database_id_ = -1;

  if (!info.lister_) {
    // It's not attached any more so remove it from the list
    beginRemoveRows(QModelIndex(), row, row);
    devices_.removeAt(row);

    foreach (const QModelIndex& idx, persistentIndexList()) {
      if (idx.row() == row)
        changePersistentIndex(idx, QModelIndex());
      else if (idx.row() > row)
        changePersistentIndex(idx, index(idx.row()-1, idx.column()));
    }

    endRemoveRows();
  } else {
    // It's still attached, set the name and icon back to what they were
    // originally
    info.friendly_name_ = info.lister_->MakeFriendlyName(info.unique_id_);
    info.icon_name_ = info.lister_->DeviceIcon(info.unique_id_);
    info.LoadIcon(info.icon_name_);

    dataChanged(index(row, 0), index(row, 0));
  }
}

void DeviceManager::SetDeviceIdentity(int row, const QString &friendly_name,
                                      const QString &icon_name) {
  DeviceInfo& info = devices_[row];
  info.friendly_name_ = friendly_name;
  info.icon_name_ = icon_name;
  info.LoadIcon(info.icon_name_);

  emit dataChanged(index(row, 0), index(row, 0));

  if (info.database_id_ != -1)
    backend_->SetDeviceIdentity(info.database_id_, friendly_name, icon_name);
}

void DeviceManager::DeviceTaskStarted(int id) {
  ConnectedDevice* device = qobject_cast<ConnectedDevice*>(sender());
  if (!device)
    return;

  for (int i=0 ; i<devices_.count() ; ++i) {
    DeviceInfo& info = devices_[i];
    if (info.device_.get() == device) {
      active_tasks_[id] = index(i);
      info.task_percentage_ = 0;
      emit dataChanged(index(i), index(i));
      return;
    }
  }
}

void DeviceManager::TasksChanged() {
  QList<TaskManager::Task> tasks = task_manager_->GetTasks();
  QList<QPersistentModelIndex> finished_tasks = active_tasks_.values();

  foreach (const TaskManager::Task& task, tasks) {
    if (!active_tasks_.contains(task.id))
      continue;

    QPersistentModelIndex index = active_tasks_[task.id];
    if (!index.isValid())
      continue;

    DeviceInfo& info = devices_[index.row()];
    if (task.progress_max)
      info.task_percentage_ = float(task.progress) / task.progress_max * 100;
    else
      info.task_percentage_ = 0;
    emit dataChanged(index, index);
    finished_tasks.removeAll(index);
  }

  foreach (const QPersistentModelIndex& index, finished_tasks) {
    if (!index.isValid())
      continue;

    DeviceInfo& info = devices_[index.row()];
    info.task_percentage_ = -1;
    emit dataChanged(index, index);
  }
}
