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

#include "filesystemdevice.h"

#include <QThread>
#include <QtDebug>

#include "core/application.h"
#include "devicelister.h"
#include "devicemanager.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "library/librarywatcher.h"

FilesystemDevice::FilesystemDevice(const QUrl& url, DeviceLister* lister,
                                   const QString& unique_id,
                                   DeviceManager* manager, Application* app,
                                   int database_id, bool first_time)
    : FilesystemMusicStorage(url.toLocalFile()),
      ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      watcher_(new LibraryWatcher),
      watcher_thread_(new QThread(this)) {
  watcher_->moveToThread(watcher_thread_);
  watcher_thread_->start(QThread::IdlePriority);

  watcher_->set_device_name(
      manager
          ->data(manager->ItemToIndex(manager->FindDeviceById(unique_id)),
                 DeviceManager::Role_FriendlyName)
          .toString());
  watcher_->set_backend(backend_.get());
  watcher_->set_task_manager(app_->task_manager());

  connect(backend_.get(),
          SIGNAL(DirectoryDiscovered(Directory, SubdirectoryList)), watcher_,
          SLOT(AddDirectory(Directory, SubdirectoryList)));
  // RemoveDirectory should be called from the sender's thread.
  connect(backend_.get(), &LibraryBackend::DirectoryDeleted, watcher_,
          &LibraryWatcher::RemoveDirectory, Qt::DirectConnection);
  connect(watcher_, SIGNAL(NewOrUpdatedSongs(SongList)), backend_.get(),
          SLOT(AddOrUpdateSongs(SongList)));
  connect(watcher_, SIGNAL(SongsMTimeUpdated(SongList)), backend_.get(),
          SLOT(UpdateMTimesOnly(SongList)));
  connect(watcher_, SIGNAL(SongsDeleted(SongList)), backend_.get(),
          SLOT(DeleteSongs(SongList)));
  connect(watcher_, SIGNAL(SubdirsDiscovered(SubdirectoryList)), backend_.get(),
          SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher_, SIGNAL(SubdirsMTimeUpdated(SubdirectoryList)),
          backend_.get(), SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher_, SIGNAL(CompilationsNeedUpdating()), backend_.get(),
          SLOT(UpdateCompilations()));
  connect(watcher_, SIGNAL(ScanStarted(int)), SIGNAL(TaskStarted(int)));
  connect(watcher_, &LibraryWatcher::Error, app, &Application::AddError);
}

void FilesystemDevice::Init() {
  InitBackendDirectory(url_.toLocalFile(), first_time_);
  model_->Init();
}

FilesystemDevice::~FilesystemDevice() {
  watcher_->Stop();
  watcher_->deleteLater();
  watcher_thread_->exit();
  watcher_thread_->wait();
}
