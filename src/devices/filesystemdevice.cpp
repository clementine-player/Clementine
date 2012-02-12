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

#include "devicelister.h"
#include "devicemanager.h"
#include "filesystemdevice.h"
#include "core/application.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "library/librarywatcher.h"

#include <QtDebug>

FilesystemDevice::FilesystemDevice(
    const QUrl& url, DeviceLister* lister,
    const QString& unique_id, DeviceManager* manager,
    Application* app,
    int database_id, bool first_time)
      : FilesystemMusicStorage(url.toLocalFile()),
        ConnectedDevice(url, lister, unique_id, manager, app, database_id, first_time),
        watcher_(new BackgroundThreadImplementation<LibraryWatcher, LibraryWatcher>(this))
{
  // Create the library watcher
  watcher_->Start(true);
  watcher_->Worker()->set_device_name(manager->data(manager->index(
      manager->FindDeviceById(unique_id)), DeviceManager::Role_FriendlyName).toString());
  watcher_->Worker()->set_backend(backend_);
  watcher_->Worker()->set_task_manager(app_->task_manager());

  // To make the connections below less verbose
  LibraryWatcher* watcher = watcher_->Worker().get();

  connect(backend_, SIGNAL(DirectoryDiscovered(Directory,SubdirectoryList)),
          watcher,  SLOT(AddDirectory(Directory,SubdirectoryList)));
  connect(backend_, SIGNAL(DirectoryDeleted(Directory)),
          watcher,  SLOT(RemoveDirectory(Directory)));
  connect(watcher,  SIGNAL(NewOrUpdatedSongs(SongList)),
          backend_, SLOT(AddOrUpdateSongs(SongList)));
  connect(watcher,  SIGNAL(SongsMTimeUpdated(SongList)),
          backend_, SLOT(UpdateMTimesOnly(SongList)));
  connect(watcher,  SIGNAL(SongsDeleted(SongList)),
          backend_, SLOT(DeleteSongs(SongList)));
  connect(watcher,  SIGNAL(SubdirsDiscovered(SubdirectoryList)),
          backend_, SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher,  SIGNAL(SubdirsMTimeUpdated(SubdirectoryList)),
          backend_, SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher,  SIGNAL(CompilationsNeedUpdating()),
          backend_, SLOT(UpdateCompilations()));
  connect(watcher, SIGNAL(ScanStarted(int)), SIGNAL(TaskStarted(int)));
}

void FilesystemDevice::Init() {
  InitBackendDirectory(url_.toLocalFile(), first_time_);
  model_->Init();
}

FilesystemDevice::~FilesystemDevice() {
}
