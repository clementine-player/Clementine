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
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "devicedatabasebackend.h"
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

DeviceManager::DeviceInfo::DeviceInfo()
    : database_id_(-1),
      transcode_mode_(MusicStorage::Transcode_Unsupported),
      transcode_format_(Song::Type_Unknown),
      task_percentage_(-1) {}

DeviceDatabaseBackend::Device DeviceManager::DeviceInfo::SaveToDb() const {
  DeviceDatabaseBackend::Device ret;
  ret.friendly_name_ = friendly_name_;
  ret.size_ = size_;
  ret.id_ = database_id_;
  ret.icon_name_ = icon_name_;
  ret.transcode_mode_ = transcode_mode_;
  ret.transcode_format_ = transcode_format_;

  QStringList unique_ids;
  for (const Backend& backend : backends_) {
    unique_ids << backend.unique_id_;
  }
  ret.unique_id_ = unique_ids.join(",");

  return ret;
}

void DeviceManager::DeviceInfo::InitFromDb(
    const DeviceDatabaseBackend::Device& dev) {
  database_id_ = dev.id_;
  friendly_name_ = dev.friendly_name_;
  size_ = dev.size_;
  transcode_mode_ = dev.transcode_mode_;
  transcode_format_ = dev.transcode_format_;

  QStringList icon_names = dev.icon_name_.split(',');
  QVariantList icons;
  for (const QString& icon_name : icon_names) {
    icons << icon_name;
  }

  LoadIcon(icons, friendly_name_);

  QStringList unique_ids = dev.unique_id_.split(',');
  for (const QString& id : unique_ids) {
    backends_ << Backend(nullptr, id);
  }
}

void DeviceManager::DeviceInfo::LoadIcon(const QVariantList& icons,
                                         const QString& name_hint) {
  if (icons.isEmpty()) {
    icon_name_ = "drive-removable-media-usb-pendrive";
    icon_ = IconLoader::Load(icon_name_, IconLoader::Base);
    return;
  }

  // Try to load the icon with that exact name first
  for (const QVariant& icon : icons) {
    if (!icon.value<QPixmap>().isNull()) {
      icon_ = QIcon(icon.value<QPixmap>());
      return;
    } else {
      icon_ = IconLoader::Load(icon.toString(), IconLoader::Base);
      if (!icon_.isNull()) {
        icon_name_ = icon.toString();
        return;
      }
    }
  }

  QString hint = QString(icons.first().toString() + name_hint).toLower();

  // If that failed than try to guess if it's a phone or ipod.  Fall back on
  // a usb memory stick icon.
  if (hint.contains("phone"))
    icon_name_ = "phone";
  else if (hint.contains("ipod") || hint.contains("apple"))
    icon_name_ = "multimedia-player-ipod-standard-monochrome";
  else
    icon_name_ = "drive-removable-media-usb-pendrive";
  icon_ = IconLoader::Load(icon_name_, IconLoader::Base);
}

const DeviceManager::DeviceInfo::Backend*
DeviceManager::DeviceInfo::BestBackend() const {
  int best_priority = -1;
  const Backend* ret = nullptr;

  for (int i = 0; i < backends_.count(); ++i) {
    if (backends_[i].lister_ &&
        backends_[i].lister_->priority() > best_priority) {
      best_priority = backends_[i].lister_->priority();
      ret = &(backends_[i]);
    }
  }

  if (!ret && !backends_.isEmpty()) return &(backends_[0]);
  return ret;
}

DeviceManager::DeviceManager(Application* app, QObject* parent)
    : QAbstractListModel(parent),
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
}

void DeviceManager::LoadAllDevices() {
  Q_ASSERT(QThread::currentThread() != qApp->thread());
  DeviceDatabaseBackend::DeviceList devices = backend_->GetAllDevices();
  for (const DeviceDatabaseBackend::Device& device : devices) {
    DeviceInfo info;
    info.InitFromDb(device);

    beginInsertRows(QModelIndex(), devices_.count(), devices_.count());
    devices_ << info;
    endInsertRows();
  }
}

int DeviceManager::rowCount(const QModelIndex&) const {
  return devices_.count();
}

QVariant DeviceManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0) return QVariant();

  const DeviceInfo& info = devices_[index.row()];

  switch (role) {
    case Qt::DisplayRole: {
      QString text;
      if (!info.friendly_name_.isEmpty())
        text = info.friendly_name_;
      else
        text = info.BestBackend()->unique_id_;

      if (info.size_)
        text = text + QString(" (%1)").arg(Utilities::PrettySize(info.size_));
      if (info.device_.get()) info.device_->Refresh();
      return text;
    }

    case Qt::DecorationRole: {
      QPixmap pixmap = info.icon_.pixmap(kDeviceIconSize);

      if (info.backends_.isEmpty() || !info.BestBackend()->lister_) {
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
      return info.BestBackend()->unique_id_;

    case Role_IconName:
      return info.icon_name_;

    case Role_Capacity:
    case MusicStorage::Role_Capacity:
      return info.size_;

    case Role_FreeSpace:
    case MusicStorage::Role_FreeSpace:
      return info.BestBackend()->lister_
                 ? info.BestBackend()->lister_->DeviceFreeSpace(
                       info.BestBackend()->unique_id_)
                 : QVariant();

    case Role_State:
      if (info.device_) return State_Connected;
      if (info.BestBackend()->lister_) {
        if (info.BestBackend()->lister_->DeviceNeedsMount(
                info.BestBackend()->unique_id_))
          return State_NotMounted;
        return State_NotConnected;
      }
      return State_Remembered;

    case Role_UpdatingPercentage:
      if (info.task_percentage_ == -1) return QVariant();
      return info.task_percentage_;

    case MusicStorage::Role_Storage:
      if (!info.device_ && info.database_id_ != -1)
        const_cast<DeviceManager*>(this)->Connect(index.row());
      if (!info.device_) return QVariant();
      return QVariant::fromValue<std::shared_ptr<MusicStorage>>(info.device_);

    case MusicStorage::Role_StorageForceConnect:
      if (!info.device_) {
        if (info.database_id_ == -1 &&
            !info.BestBackend()->lister_->DeviceNeedsMount(
                info.BestBackend()->unique_id_)) {
          if (info.BestBackend()->lister_->AskForScan(
                  info.BestBackend()->unique_id_)) {
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

        const_cast<DeviceManager*>(this)->Connect(index.row());
      }
      if (!info.device_) return QVariant();
      return QVariant::fromValue<std::shared_ptr<MusicStorage>>(info.device_);

    case Role_MountPath: {
      if (!info.device_) return QVariant();

      QString ret = info.device_->url().path();
#ifdef Q_OS_WIN32
      if (ret.startsWith('/')) ret.remove(0, 1);
#endif
      return QDir::toNativeSeparators(ret);
    }

    case Role_TranscodeMode:
      return info.transcode_mode_;

    case Role_TranscodeFormat:
      return info.transcode_format_;

    case Role_SongCount:
      if (!info.device_) return QVariant();
      return info.device_->song_count();

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

int DeviceManager::FindDeviceById(const QString& id) const {
  for (int i = 0; i < devices_.count(); ++i) {
    for (const DeviceInfo::Backend& backend : devices_[i].backends_) {
      if (backend.unique_id_ == id) return i;
    }
  }
  return -1;
}

int DeviceManager::FindDeviceByUrl(const QList<QUrl>& urls) const {
  if (urls.isEmpty()) return -1;

  for (int i = 0; i < devices_.count(); ++i) {
    for (const DeviceInfo::Backend& backend : devices_[i].backends_) {
      if (!backend.lister_) continue;

      QList<QUrl> device_urls =
          backend.lister_->MakeDeviceUrls(backend.unique_id_);
      for (const QUrl& url : device_urls) {
        if (urls.contains(url)) return i;
      }
    }
  }
  return -1;
}

void DeviceManager::PhysicalDeviceAdded(const QString& id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  qLog(Info) << "Device added:" << id;

  // Do we have this device already?
  int i = FindDeviceById(id);
  if (i != -1) {
    DeviceInfo& info = devices_[i];
    for (int backend_index = 0; backend_index < info.backends_.count();
         ++backend_index) {
      if (info.backends_[backend_index].unique_id_ == id) {
        info.backends_[backend_index].lister_ = lister;
        break;
      }
    }

    emit dataChanged(index(i, 0), index(i, 0));
  } else {
    // Check if we have another device with the same URL
    i = FindDeviceByUrl(lister->MakeDeviceUrls(id));
    if (i != -1) {
      // Add this device's lister to the existing device
      DeviceInfo& info = devices_[i];
      info.backends_ << DeviceInfo::Backend(lister, id);

      // If the user hasn't saved the device in the DB yet then overwrite the
      // device's name and icon etc.
      if (info.database_id_ == -1 && info.BestBackend()->lister_ == lister) {
        info.friendly_name_ = lister->MakeFriendlyName(id);
        info.size_ = lister->DeviceCapacity(id);
        info.LoadIcon(lister->DeviceIcons(id), info.friendly_name_);
      }

      emit dataChanged(index(i, 0), index(i, 0));
    } else {
      // It's a completely new device
      DeviceInfo info;
      info.backends_ << DeviceInfo::Backend(lister, id);
      info.friendly_name_ = lister->MakeFriendlyName(id);
      info.size_ = lister->DeviceCapacity(id);
      info.LoadIcon(lister->DeviceIcons(id), info.friendly_name_);

      beginInsertRows(QModelIndex(), devices_.count(), devices_.count());
      devices_ << info;
      endInsertRows();
    }
  }
}

void DeviceManager::PhysicalDeviceRemoved(const QString& id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());

  qLog(Info) << "Device removed:" << id;

  int i = FindDeviceById(id);
  if (i == -1) {
    // Shouldn't happen
    return;
  }

  DeviceInfo& info = devices_[i];

  if (info.database_id_ != -1) {
    // Keep the structure around, but just "disconnect" it
    for (int backend_index = 0; backend_index < info.backends_.count();
         ++backend_index) {
      if (info.backends_[backend_index].unique_id_ == id) {
        info.backends_[backend_index].lister_ = nullptr;
        break;
      }
    }

    if (info.device_ && info.device_->lister() == lister) info.device_.reset();

    if (!info.device_) emit DeviceDisconnected(i);

    emit dataChanged(index(i, 0), index(i, 0));
  } else {
    // If this was the last lister for the device then remove it from the model
    for (int backend_index = 0; backend_index < info.backends_.count();
         ++backend_index) {
      if (info.backends_[backend_index].unique_id_ == id) {
        info.backends_.removeAt(backend_index);
        break;
      }
    }

    if (info.backends_.isEmpty()) {
      beginRemoveRows(QModelIndex(), i, i);
      devices_.removeAt(i);

      for (const QModelIndex& idx : persistentIndexList()) {
        if (idx.row() == i)
          changePersistentIndex(idx, QModelIndex());
        else if (idx.row() > i)
          changePersistentIndex(idx, index(idx.row() - 1, idx.column()));
      }

      endRemoveRows();
    }
  }
}

void DeviceManager::PhysicalDeviceChanged(const QString& id) {
  DeviceLister* lister = qobject_cast<DeviceLister*>(sender());
  Q_UNUSED(lister);

  int i = FindDeviceById(id);
  if (i == -1) {
    // Shouldn't happen
    return;
  }

  // TODO
}

std::shared_ptr<ConnectedDevice> DeviceManager::Connect(int row) {
  DeviceInfo& info = devices_[row];
  if (info.device_)  // Already connected
    return info.device_;

  std::shared_ptr<ConnectedDevice> ret;

  if (!info.BestBackend()->lister_)  // Not physically connected
    return ret;

  if (info.BestBackend()->lister_->DeviceNeedsMount(
          info.BestBackend()->unique_id_)) {
    // Mount the device
    info.BestBackend()->lister_->MountDevice(info.BestBackend()->unique_id_);
    return ret;
  }

  bool first_time = (info.database_id_ == -1);
  if (first_time) {
    // We haven't stored this device in the database before
    info.database_id_ = backend_->AddDevice(info.SaveToDb());
  }

  // Get the device URLs
  QList<QUrl> urls = info.BestBackend()->lister_->MakeDeviceUrls(
      info.BestBackend()->unique_id_);
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
      Q_ARG(DeviceLister*, info.BestBackend()->lister_),
      Q_ARG(QString, info.BestBackend()->unique_id_),
      Q_ARG(DeviceManager*, this), Q_ARG(Application*, app_),
      Q_ARG(int, info.database_id_), Q_ARG(bool, first_time));
  ret.reset(static_cast<ConnectedDevice*>(instance));

  if (!ret) {
    qLog(Warning) << "Could not create device for" << device_url.toString();
  } else {
    ret->Init();

    info.device_ = ret;
    emit dataChanged(index(row), index(row));
    connect(info.device_.get(), SIGNAL(TaskStarted(int)),
            SLOT(DeviceTaskStarted(int)));
    connect(info.device_.get(), SIGNAL(SongCountUpdated(int)),
            SLOT(DeviceSongCountUpdated(int)));
  }

  emit DeviceConnected(row);

  return ret;
}

std::shared_ptr<ConnectedDevice> DeviceManager::GetConnectedDevice(
    int row) const {
  return devices_[row].device_;
}

int DeviceManager::GetDatabaseId(int row) const {
  return devices_[row].database_id_;
}

DeviceLister* DeviceManager::GetLister(int row) const {
  return devices_[row].BestBackend()->lister_;
}

void DeviceManager::Disconnect(int row) {
  DeviceInfo& info = devices_[row];
  if (!info.device_)  // Already disconnected
    return;

  info.device_.reset();
  emit DeviceDisconnected(row);
  emit dataChanged(index(row), index(row));
}

void DeviceManager::Forget(int row) {
  DeviceInfo& info = devices_[row];
  if (info.database_id_ == -1) return;

  if (info.device_) Disconnect(row);

  backend_->RemoveDevice(info.database_id_);
  info.database_id_ = -1;

  if (!info.BestBackend()->lister_) {
    // It's not attached any more so remove it from the list
    beginRemoveRows(QModelIndex(), row, row);
    devices_.removeAt(row);

    for (const QModelIndex& idx : persistentIndexList()) {
      if (idx.row() == row)
        changePersistentIndex(idx, QModelIndex());
      else if (idx.row() > row)
        changePersistentIndex(idx, index(idx.row() - 1, idx.column()));
    }

    endRemoveRows();
  } else {
    // It's still attached, set the name and icon back to what they were
    // originally
    const QString id = info.BestBackend()->unique_id_;

    info.friendly_name_ = info.BestBackend()->lister_->MakeFriendlyName(id);
    info.LoadIcon(info.BestBackend()->lister_->DeviceIcons(id),
                  info.friendly_name_);

    dataChanged(index(row, 0), index(row, 0));
  }
}

void DeviceManager::SetDeviceOptions(int row, const QString& friendly_name,
                                     const QString& icon_name,
                                     MusicStorage::TranscodeMode mode,
                                     Song::FileType format) {
  DeviceInfo& info = devices_[row];
  info.friendly_name_ = friendly_name;
  info.LoadIcon(QVariantList() << icon_name, friendly_name);
  info.transcode_mode_ = mode;
  info.transcode_format_ = format;

  emit dataChanged(index(row, 0), index(row, 0));

  if (info.database_id_ != -1)
    backend_->SetDeviceOptions(info.database_id_, friendly_name, icon_name,
                               mode, format);
}

void DeviceManager::DeviceTaskStarted(int id) {
  ConnectedDevice* device = qobject_cast<ConnectedDevice*>(sender());
  if (!device) return;

  for (int i = 0; i < devices_.count(); ++i) {
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
  QList<TaskManager::Task> tasks = app_->task_manager()->GetTasks();
  QList<QPersistentModelIndex> finished_tasks = active_tasks_.values();

  for (const TaskManager::Task& task : tasks) {
    if (!active_tasks_.contains(task.id)) continue;

    QPersistentModelIndex index = active_tasks_[task.id];
    if (!index.isValid()) continue;

    DeviceInfo& info = devices_[index.row()];
    if (task.progress_max)
      info.task_percentage_ = float(task.progress) / task.progress_max * 100;
    else
      info.task_percentage_ = 0;
    emit dataChanged(index, index);
    finished_tasks.removeAll(index);
  }

  for (const QPersistentModelIndex& index : finished_tasks) {
    if (!index.isValid()) continue;

    DeviceInfo& info = devices_[index.row()];
    info.task_percentage_ = -1;
    emit dataChanged(index, index);

    active_tasks_.remove(active_tasks_.key(index));
  }
}

void DeviceManager::UnmountAsync(int row) {
  Q_ASSERT(QMetaObject::invokeMethod(this, "Unmount", Q_ARG(int, row)));
}

void DeviceManager::Unmount(int row) {
  DeviceInfo& info = devices_[row];
  if (info.database_id_ != -1 && !info.device_) return;

  if (info.device_) Disconnect(row);

  if (info.BestBackend()->lister_)
    info.BestBackend()->lister_->UnmountDevice(info.BestBackend()->unique_id_);
}

void DeviceManager::DeviceSongCountUpdated(int count) {
  ConnectedDevice* device = qobject_cast<ConnectedDevice*>(sender());
  if (!device) return;

  int row = FindDeviceById(device->unique_id());
  if (row == -1) return;

  emit dataChanged(index(row), index(row));
}
