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

#include "connecteddevice.h"
#include "devicelister.h"
#include "devicemanager.h"
#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <QtDebug>

ConnectedDevice::ConnectedDevice(const QUrl& url, DeviceLister* lister,
                                 const QString& unique_id,
                                 DeviceManager* manager, Application* app,
                                 int database_id, bool first_time)
    : QObject(manager),
      app_(app),
      url_(url),
      first_time_(first_time),
      lister_(lister),
      unique_id_(unique_id),
      database_id_(database_id),
      manager_(manager),
      backend_(nullptr),
      model_(nullptr),
      song_count_(0) {
  qLog(Info) << "connected" << url << unique_id << first_time;

  // Create the backend in the database thread.
  backend_ = new LibraryBackend();
  backend_->moveToThread(app_->database()->thread());

  connect(backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(BackendTotalSongCountUpdated(int)));

  backend_->Init(app_->database(), QString("device_%1_songs").arg(database_id),
                 QString("device_%1_directories").arg(database_id),
                 QString("device_%1_subdirectories").arg(database_id),
                 QString("device_%1_fts").arg(database_id));

  // Create the model
  model_ = new LibraryModel(backend_, app_, this);
}

ConnectedDevice::~ConnectedDevice() { backend_->deleteLater(); }

void ConnectedDevice::InitBackendDirectory(const QString& mount_point,
                                           bool first_time, bool rewrite_path) {
  if (first_time || backend_->GetAllDirectories().isEmpty()) {
    backend_->AddDirectory(mount_point);
  } else {
    if (rewrite_path) {
      // This is a bit of a hack.  The device might not be mounted at the same
      // path each time, so if it's different we have to munge all the paths in
      // the database to fix it.  This can be done entirely in sqlite so it's
      // relatively fast...

      // Get the directory it was mounted at last time.  Devices only have one
      // directory (the root).
      Directory dir = backend_->GetAllDirectories()[0];
      if (dir.path != mount_point) {
        // The directory is different, commence the munging.
        qLog(Info) << "Changing path from" << dir.path << "to" << mount_point;
        backend_->ChangeDirPath(dir.id, dir.path, mount_point);
      }
    }

    // Load the directory properly now
    backend_->LoadDirectoriesAsync();
  }
}

void ConnectedDevice::ConnectAsync() { emit ConnectFinished(unique_id_, true); }

void ConnectedDevice::Eject() {
  DeviceInfo* info = manager_->FindDeviceById(unique_id_);
  if (!info) return;
  QModelIndex idx = manager_->ItemToIndex(info);
  if (!idx.isValid()) return;
  manager_->UnmountAsync(idx);
}

void ConnectedDevice::FinishCopy(bool) {
  lister_->UpdateDeviceFreeSpace(unique_id_);
}

void ConnectedDevice::FinishDelete(bool) {
  lister_->UpdateDeviceFreeSpace(unique_id_);
}

MusicStorage::TranscodeMode ConnectedDevice::GetTranscodeMode() const {
  DeviceInfo* info = manager_->FindDeviceById(unique_id_);
  if (!info) return MusicStorage::TranscodeMode();

  QModelIndex idx = manager_->ItemToIndex(info);
  if (!idx.isValid()) return MusicStorage::TranscodeMode();

  return MusicStorage::TranscodeMode(
      idx.data(DeviceManager::Role_TranscodeMode).toInt());
}

Song::FileType ConnectedDevice::GetTranscodeFormat() const {
  DeviceInfo* info = manager_->FindDeviceById(unique_id_);
  if (!info) return Song::Type_Unknown;

  QModelIndex idx = manager_->ItemToIndex(info);
  if (!idx.isValid()) return Song::Type_Unknown;

  return Song::FileType(idx.data(DeviceManager::Role_TranscodeFormat).toInt());
}

void ConnectedDevice::BackendTotalSongCountUpdated(int count) {
  song_count_ = count;
  emit SongCountUpdated(count);
}
