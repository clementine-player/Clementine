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

#include "devicemanager.h"

#include <memory>

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QUrl>

#include "config.h"
#include "core/application.h"
#include "core/concurrentrun.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/musicstorage.h"
#include "core/simpletreemodel.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "devicedatabasebackend.h"
#include "deviceinfo.h"
#include "devicekitlister.h"
#include "devicestatefiltermodel.h"
#include "filesystemdevice.h"
#include "ui/iconloader.h"

#ifdef HAVE_AUDIOCD
#include "cddadevice.h"
#include "cddalister.h"
#endif

#if defined(Q_OS_DARWIN) and defined(HAVE_LIBMTP)
#include "macdevicelister.h"
#endif
#ifdef HAVE_LIBGPOD
#include "gpoddevice.h"
#endif
#ifdef HAVE_GIO
#include "giolister.h"
#endif
#ifdef HAVE_LIBMTP
#include "mtpdevice.h"
#endif
#ifdef HAVE_UDISKS2
#include "udisks2lister.h"
#endif

using std::bind;

const int DeviceManager::kDeviceIconSize = 32;
const int DeviceManager::kDeviceIconOverlaySize = 16;

DeviceManager::DeviceManager(Application* app, QObject* parent)
    : SimpleTreeModel<DeviceInfo>(new DeviceInfo(this), parent),
      app_(app),
      not_connected_overlay_(
          IconLoader::Load("edit-delete", IconLoader::Base)) {
  thread_pool_.setMaxThreadCount(1);
  connect(app_->task_manager(), SIGNAL(TasksChanged()), SLOT(TasksChanged()));

  // Create the backend in the database thread
  backend_ = new DeviceDatabaseBackend;
  backend_->moveToThread(app_->database()->thread());
  backend_->Init(app_->database());

  // This reads from the database and contends on the database mutex, which can
  // be very slow on startup.
  ConcurrentRun::Run<void>(&thread_pool_,
                           bind(&DeviceManager::LoadAllDevices, this));

  // This proxy model only shows connected devices
  connected_devices_model_ = new DeviceStateFilterModel(this);
  connected_devices_model_->setSourceModel(this);

// CD devices are detected via the DiskArbitration framework instead on Darwin.
#if defined(HAVE_AUDIOCD) && !defined(Q_OS_DARWIN)
  AddLister(new CddaLister);
#endif
#ifdef HAVE_DEVICEKIT
  AddLister(new DeviceKitLister);
#endif
#ifdef HAVE_UDISKS2
  AddLister(new Udisks2Lister);
#endif
#ifdef HAVE_GIO
  AddLister(new GioLister);
#endif
#if defined(Q_OS_DARWIN) and defined(HAVE_LIBMTP)
  AddLister(new MacDeviceLister);
#endif

  AddDeviceClass<FilesystemDevice>();

#ifdef HAVE_AUDIOCD
  AddDeviceClass<CddaDevice>();
#endif

#ifdef HAVE_LIBGPOD
  AddDeviceClass<GPodDevice>();
#endif

#ifdef HAVE_LIBMTP
  AddDeviceClass<MtpDevice>();
#endif
}

DeviceManager::~DeviceManager() {
  for (DeviceLister* lister : listers_) {
    lister->ShutDown();
    delete lister;
  }

  backend_->deleteLater();
  delete root_;
}

void DeviceManager::LoadAllDevices() {
  Q_ASSERT(QThread::currentThread() != qApp->thread());
  DeviceDatabaseBackend::DeviceList devices = backend_->GetAllDevices();
  for (const DeviceDatabaseBackend::Device& device : devices) {
    DeviceInfo* info = new DeviceInfo(DeviceInfo::Type_Device, root_);
    info->InitFromDb(device);

    beginInsertRows(ItemToIndex(root_), devices_.count(), devices_.count());
    devices_ << info;
    endInsertRows();
  }
}

QVariant DeviceManager::data(const QModelIndex& idx, int role) const {
  if (!idx.isValid() || idx.column() != 0) return QVariant();

  DeviceInfo* info = IndexToItem(idx);
  if (!info) return QVariant();

  switch (role) {
    case Qt::DisplayRole: {
      QString text;
      if (!info->friendly_name_.isEmpty())
        text = info->friendly_name_;
      else if (info->BestBackend())
        text = info->BestBackend()->unique_id_;

      if (info->size_)
        text = text + QString(" (%1)").arg(Utilities::PrettySize(info->size_));
      if (info->device_.get()) info->device_->Refresh();
      return text;
    }

    case Qt::DecorationRole: {
      QPixmap pixmap = info->icon_.pixmap(kDeviceIconSize);

      if (info->backends_.isEmpty() || !info->BestBackend()->lister_) {
        // Disconnected but remembered
        QPainter p(&pixmap);
        p.drawPixmap(kDeviceIconSize - kDeviceIconOverlaySize,
                     kDeviceIconSize - kDeviceIconOverlaySize,
                     not_connected_overlay_.pixmap(kDeviceIconOverlaySize));
      }

      return pixmap;
    }

    case Role_FriendlyName:
      return info->friendly_name_;

    case Role_UniqueId:
      if (!info->BestBackend()) return QString();
      return info->BestBackend()->unique_id_;

    case Role_IconName:
      return info->icon_name_;

    case Role_Capacity:
    case MusicStorage::Role_Capacity:
      return info->size_;

    case Role_FreeSpace:
    case MusicStorage::Role_FreeSpace:
      return info->BestBackend() && info->BestBackend()->lister_
                 ? info->BestBackend()->lister_->DeviceFreeSpace(
                       info->BestBackend()->unique_id_)
                 : QVariant();

    case Role_State:
      if (info->device_) return State_Connected;
      if (info->BestBackend() && info->BestBackend()->lister_) {
        if (info->BestBackend()->lister_->DeviceNeedsMount(
                info->BestBackend()->unique_id_))
          return State_NotMounted;
        return State_NotConnected;
      }
      return State_Remembered;

    case Role_UpdatingPercentage:
      if (info->task_percentage_ == -1) return QVariant();
      return info->task_percentage_;

    case MusicStorage::Role_Storage:
      if (!info->device_ && info->database_id_ != -1)
        const_cast<DeviceManager*>(this)->Connect(info);
      if (!info->device_) return QVariant();
      return QVariant::fromValue<std::shared_ptr<MusicStorage>>(info->device_);

    case MusicStorage::Role_StorageForceConnect:
      if (!info->BestBackend()) return QVariant();
      if (!info->device_) {
        if (info->database_id_ == -1 &&
            !info->BestBackend()->lister_->DeviceNeedsMount(
                info->BestBackend()->unique_id_)) {
          if (info->BestBackend()->lister_->AskForScan(
                  info->BestBackend()->unique_id_)) {
            std::unique_ptr<QMessageBox> dialog(new QMessageBox(
                QMessageBox::Information, tr("Connect device"),
                tr("This is the first time you have connected this device.  "
                   "Clementine will now scan the device to find music files - "
                   "this may take some time."),
                QMessageBox::Cancel));
            QPushButton* connect = dialog->addButton(tr("Connect device"),
                                                     QMessageBox::AcceptRole);
            dialog->exec();

            if (dialog->clickedButton() != connect) return QVariant();
          }
        }

        const_cast<DeviceManager*>(this)->Connect(info);
      }
      if (!info->device_) return QVariant();
      return QVariant::fromValue<std::shared_ptr<MusicStorage>>(info->device_);

    case Role_MountPath: {
      if (!info->device_) return QVariant();

      QString ret = info->device_->url().path();
#ifdef Q_OS_WIN32
      if (ret.startsWith('/')) ret.remove(0, 1);
#endif
      return QDir::toNativeSeparators(ret);
    }

    case Role_TranscodeMode:
      return info->transcode_mode_;

    case Role_TranscodeFormat:
      return info->transcode_format_;

    case Role_SongCount:
      if (!info->device_) return QVariant();
      return info->device_->song_count();

    default:
      return QVariant();
  }
}

void DeviceManager::AddLister(DeviceLister* lister) {
  listers_ << lister;
  connect(lister, SIGNAL(DeviceAdded(QString)),
          SLOT(PhysicalDeviceAdded(QString)));
  connect(lister, SIGNAL(DeviceRemoved(QString)),
          SLOT(PhysicalDeviceRemoved(QString)));
  connect(lister, SIGNAL(DeviceChanged(QString)),
          SLOT(PhysicalDeviceChanged(QString)));

  lister->Start();
}

DeviceInfo* DeviceManager::FindDeviceById(const QString& id) const {
  for (int i = 0; i < devices_.count(); ++i) {
    for (const DeviceInfo::Backend& backend : devices_[i]->backends_) {
      if (backend.unique_id_ == id) return devices_[i];
    }
  }
  return nullptr;
}

DeviceInfo* DeviceManager::FindDeviceByUrl(const QList<QUrl>& urls) const {
  if (urls.isEmpty()) return nullptr;

  for (int i = 0; i < devices_.count(); ++i) {
    for (const DeviceInfo::Backend& backend : devices_[i]->backends_) {
      if (!backend.lister_) continue;

      QList<QUrl> device_urls =
          backend.lister_->MakeDeviceUrls(backend.unique_id_);
      for (const QUrl& url : device_urls) {
        if (urls.contains(url)) return devices_[i];
      }
    }
  }
  return nullptr;
}

void DeviceManager::PhysicalDeviceAdded(const QString& id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  qLog(Info) << "Device added:" << id;

  // Do we have this device already?
  DeviceInfo* info = FindDeviceById(id);
  if (info) {
    for (int backend_index = 0; backend_index < info->backends_.count();
         ++backend_index) {
      if (info->backends_[backend_index].unique_id_ == id) {
        info->backends_[backend_index].lister_ = lister;
        break;
      }
    }
    QModelIndex idx = ItemToIndex(info);
    if (idx.isValid()) emit dataChanged(idx, idx);
  } else {
    // Check if we have another device with the same URL
    info = FindDeviceByUrl(lister->MakeDeviceUrls(id));
    if (info) {
      // Add this device's lister to the existing device
      info->backends_ << DeviceInfo::Backend(lister, id);

      // If the user hasn't saved the device in the DB yet then overwrite the
      // device's name and icon etc.
      if (info->database_id_ == -1 && info->BestBackend()->lister_ == lister) {
        info->friendly_name_ = lister->MakeFriendlyName(id);
        info->size_ = lister->DeviceCapacity(id);
        info->LoadIcon(lister->DeviceIcons(id), info->friendly_name_);
      }

      QModelIndex idx = ItemToIndex(info);
      if (idx.isValid()) emit dataChanged(idx, idx);
    } else {
      // It's a completely new device
      DeviceInfo* info = new DeviceInfo(DeviceInfo::Type_Device, root_);
      info->backends_ << DeviceInfo::Backend(lister, id);
      info->friendly_name_ = lister->MakeFriendlyName(id);
      info->size_ = lister->DeviceCapacity(id);
      info->LoadIcon(lister->DeviceIcons(id), info->friendly_name_);

      beginInsertRows(ItemToIndex(root_), devices_.count(), devices_.count());
      devices_ << info;
      endInsertRows();
    }
  }
}

void DeviceManager::PhysicalDeviceRemoved(const QString& id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  qLog(Info) << "Device removed:" << id;

  DeviceInfo* info = FindDeviceById(id);
  if (!info) {
    // Shouldn't happen
    return;
  }

  QModelIndex idx = ItemToIndex(info);
  if (!idx.isValid()) return;

  if (info->database_id_ != -1) {
    // Keep the structure around, but just "disconnect" it
    for (int backend_index = 0; backend_index < info->backends_.count();
         ++backend_index) {
      if (info->backends_[backend_index].unique_id_ == id) {
        info->backends_[backend_index].lister_ = nullptr;
        break;
      }
    }

    if (info->device_ && info->device_->lister() == lister)
      info->device_.reset();

    if (!info->device_) emit DeviceDisconnected(idx);

    emit dataChanged(idx, idx);
  } else {
    // If this was the last lister for the device then remove it from the model
    for (int backend_index = 0; backend_index < info->backends_.count();
         ++backend_index) {
      if (info->backends_[backend_index].unique_id_ == id) {
        info->backends_.removeAt(backend_index);
        break;
      }
    }

    if (info->backends_.isEmpty()) {
      beginRemoveRows(ItemToIndex(root_), idx.row(), idx.row());
      devices_.removeAll(info);
      root_->Delete(info->row);
      endRemoveRows();
    }
  }
}

void DeviceManager::PhysicalDeviceChanged(const QString& id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());
  Q_UNUSED(lister);

  DeviceInfo* info = FindDeviceById(id);
  if (!info) {
    // Shouldn't happen
    return;
  }

  // TODO
}

std::shared_ptr<ConnectedDevice> DeviceManager::Connect(QModelIndex idx) {
  DeviceInfo* info = IndexToItem(idx);
  if (!info) return std::shared_ptr<ConnectedDevice>();

  return Connect(info);
}

std::shared_ptr<ConnectedDevice> DeviceManager::Connect(DeviceInfo* info) {
  std::shared_ptr<ConnectedDevice> ret;

  if (!info) return ret;
  if (info->device_)  // Already connected
    return info->device_;

  if (!info->BestBackend() ||
      !info->BestBackend()->lister_)  // Not physically connected
    return ret;

  if (info->BestBackend()->lister_->DeviceNeedsMount(
          info->BestBackend()->unique_id_)) {
    // Mount the device
    info->BestBackend()->lister_->MountDevice(info->BestBackend()->unique_id_);
    return ret;
  }

  bool first_time = (info->database_id_ == -1);
  if (first_time) {
    // We haven't stored this device in the database before
    info->database_id_ = backend_->AddDevice(info->SaveToDb());
  }

  // Get the device URLs
  QList<QUrl> urls = info->BestBackend()->lister_->MakeDeviceUrls(
      info->BestBackend()->unique_id_);
  if (urls.isEmpty()) return ret;

  // Take the first URL that we have a handler for
  QUrl device_url;
  for (const QUrl& url : urls) {
    qLog(Info) << "Connecting" << url;

    // Find a device class for this URL's scheme
    if (device_classes_.contains(url.scheme())) {
      device_url = url;
      break;
    }

    // If we get here it means that this URL scheme wasn't supported.  If it
    // was "ipod" or "mtp" then the user compiled out support and the device
    // won't work properly.
    if (url.scheme() == "mtp" || url.scheme() == "gphoto2") {
      if (QMessageBox::critical(
              nullptr, tr("This device will not work properly"),
              tr("This is an MTP device, but you compiled Clementine without "
                 "libmtp support.") +
                  "  " + tr("If you continue, this device will work slowly and "
                            "songs copied to it may not work."),
              QMessageBox::Abort, QMessageBox::Ignore) == QMessageBox::Abort)
        return ret;
    }

    if (url.scheme() == "ipod") {
      if (QMessageBox::critical(
              nullptr, tr("This device will not work properly"),
              tr("This is an iPod, but you compiled Clementine without libgpod "
                 "support.") +
                  "  " + tr("If you continue, this device will work slowly and "
                            "songs copied to it may not work."),
              QMessageBox::Abort, QMessageBox::Ignore) == QMessageBox::Abort)
        return ret;
    }
  }

  if (device_url.isEmpty()) {
    // Munge the URL list into a string list
    QStringList url_strings;
    for (const QUrl& url : urls) {
      url_strings << url.toString();
    }

    app_->AddError(tr("This type of device is not supported: %1")
                       .arg(url_strings.join(", ")));
    return ret;
  }

  QMetaObject meta_object = device_classes_.value(device_url.scheme());
  QObject* instance = meta_object.newInstance(
      Q_ARG(QUrl, device_url),
      Q_ARG(DeviceLister*, info->BestBackend()->lister_),
      Q_ARG(QString, info->BestBackend()->unique_id_),
      Q_ARG(DeviceManager*, this), Q_ARG(Application*, app_),
      Q_ARG(int, info->database_id_), Q_ARG(bool, first_time));
  ret.reset(static_cast<ConnectedDevice*>(instance));

  if (!ret) {
    qLog(Warning) << "Could not create device for" << device_url.toString();
  } else {
    ret->Init();

    info->device_ = ret;
    QModelIndex idx = ItemToIndex(info);
    if (!idx.isValid()) return ret;
    emit dataChanged(idx, idx);
    connect(info->device_.get(), SIGNAL(TaskStarted(int)),
            SLOT(DeviceTaskStarted(int)));
    connect(info->device_.get(), SIGNAL(SongCountUpdated(int)),
            SLOT(DeviceSongCountUpdated(int)));
    connect(info->device_.get(), SIGNAL(ConnectFinished(const QString&, bool)),
            SLOT(DeviceConnectFinished(const QString&, bool)));
    ret->ConnectAsync();
  }

  return ret;
}

void DeviceManager::DeviceConnectFinished(const QString& id, bool success) {
  DeviceInfo* info = FindDeviceById(id);
  if (!info) return;

  QModelIndex idx = ItemToIndex(info);
  if (!idx.isValid()) return;

  if (success) {
    emit DeviceConnected(idx);
  } else {
    info->device_.reset();
  }
}

std::shared_ptr<ConnectedDevice> DeviceManager::GetConnectedDevice(
    QModelIndex idx) const {
  DeviceInfo* info = IndexToItem(idx);
  if (!info) return std::shared_ptr<ConnectedDevice>();
  return info->device_;
}

std::shared_ptr<ConnectedDevice> DeviceManager::GetConnectedDevice(
    DeviceInfo* info) const {
  if (!info) return std::shared_ptr<ConnectedDevice>();
  return info->device_;
}

int DeviceManager::GetDatabaseId(const QModelIndex& idx) const {
  if (!idx.isValid()) return -1;
  DeviceInfo* info = IndexToItem(idx);
  if (!info) return -1;
  return info->database_id_;
}

DeviceLister* DeviceManager::GetLister(QModelIndex idx) const {
  if (!idx.isValid()) return nullptr;

  DeviceInfo* info = IndexToItem(idx);
  if (!info || !info->BestBackend()) return nullptr;
  return info->BestBackend()->lister_;
}

void DeviceManager::Disconnect(QModelIndex idx) {
  if (!idx.isValid()) return;
  DeviceInfo* info = IndexToItem(idx);
  if (!info || !info->device_)  // Already disconnected
    return;

  info->device_.reset();
  emit DeviceDisconnected(idx);
  emit dataChanged(idx, idx);
}

void DeviceManager::Forget(QModelIndex idx) {
  if (!idx.isValid()) return;
  DeviceInfo* info = IndexToItem(idx);
  if (!info) return;
  if (info->database_id_ == -1) return;

  if (info->device_) Disconnect(idx);

  backend_->RemoveDevice(info->database_id_);
  info->database_id_ = -1;

  if (!info->BestBackend() ||
      (info->BestBackend() && !info->BestBackend()->lister_)) {
    // It's not attached any more so remove it from the list
    beginRemoveRows(ItemToIndex(root_), idx.row(), idx.row());
    devices_.removeAll(info);
    root_->Delete(info->row);
    endRemoveRows();
  } else {
    // It's still attached, set the name and icon back to what they were
    // originally
    const QString id = info->BestBackend()->unique_id_;

    info->friendly_name_ = info->BestBackend()->lister_->MakeFriendlyName(id);
    info->LoadIcon(info->BestBackend()->lister_->DeviceIcons(id),
                   info->friendly_name_);

    dataChanged(idx, idx);
  }
}

void DeviceManager::SetDeviceOptions(QModelIndex idx,
                                     const QString& friendly_name,
                                     const QString& icon_name,
                                     MusicStorage::TranscodeMode mode,
                                     Song::FileType format) {
  if (!idx.isValid()) return;
  DeviceInfo* info = IndexToItem(idx);
  if (!info) return;
  info->friendly_name_ = friendly_name;
  info->LoadIcon(QVariantList() << icon_name, friendly_name);
  info->transcode_mode_ = mode;
  info->transcode_format_ = format;

  emit dataChanged(idx, idx);

  if (info->database_id_ != -1)
    backend_->SetDeviceOptions(info->database_id_, friendly_name, icon_name,
                               mode, format);
}

void DeviceManager::DeviceTaskStarted(int id) {
  ConnectedDevice* device = qobject_cast<ConnectedDevice*>(sender());
  if (!device) return;

  for (int i = 0; i < devices_.count(); ++i) {
    DeviceInfo* info = devices_[i];
    if (info->device_.get() == device) {
      QModelIndex index = ItemToIndex(info);
      if (!index.isValid()) continue;
      active_tasks_[id] = index;
      info->task_percentage_ = 0;
      emit dataChanged(index, index);
      return;
    }
  }
}

void DeviceManager::TasksChanged() {
  QList<TaskManager::Task> tasks = app_->task_manager()->GetTasks();
  QList<QPersistentModelIndex> finished_tasks = active_tasks_.values();

  for (const TaskManager::Task& task : tasks) {
    if (!active_tasks_.contains(task.id)) continue;

    QPersistentModelIndex idx = active_tasks_[task.id];
    if (!idx.isValid()) continue;

    DeviceInfo* info = IndexToItem(idx);
    if (task.progress_max)
      info->task_percentage_ = float(task.progress) / task.progress_max * 100;
    else
      info->task_percentage_ = 0;
    emit dataChanged(idx, idx);
    finished_tasks.removeAll(idx);
  }

  for (const QPersistentModelIndex& idx : finished_tasks) {
    if (!idx.isValid()) continue;

    DeviceInfo* info = IndexToItem(idx);
    if (!info) continue;
    info->task_percentage_ = -1;
    emit dataChanged(idx, idx);

    active_tasks_.remove(active_tasks_.key(idx));
  }
}

void DeviceManager::UnmountAsync(QModelIndex idx) {
  Q_ASSERT(QMetaObject::invokeMethod(this, "Unmount", Q_ARG(QModelIndex, idx)));
}

void DeviceManager::Unmount(QModelIndex idx) {
  if (!idx.isValid()) return;
  DeviceInfo* info = IndexToItem(idx);
  if (!info) return;
  if (info->database_id_ != -1 && !info->device_) return;

  if (info->device_) Disconnect(idx);

  if (info->BestBackend()->lister_)
    info->BestBackend()->lister_->UnmountDevice(
        info->BestBackend()->unique_id_);
}

void DeviceManager::DeviceSongCountUpdated(int count) {
  ConnectedDevice* device = qobject_cast<ConnectedDevice*>(sender());
  if (!device) return;

  DeviceInfo* info = FindDeviceById(device->unique_id());
  if (!info) return;

  QModelIndex idx = ItemToIndex(info);
  if (!idx.isValid()) return;

  emit dataChanged(idx, idx);
}

void DeviceManager::LazyPopulate(DeviceInfo* parent, bool signal) {
  if (parent->lazy_loaded) return;
  parent->lazy_loaded = true;
}
