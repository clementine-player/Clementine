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

#include "devicelister.h"
#include "devicemanager.h"
#include "filesystemdevice.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "library/librarywatcher.h"

#include <QtDebug>

FilesystemDevice::FilesystemDevice(
    const QString& mount_point, DeviceLister* lister, const QString& unique_id,
    DeviceManager* manager, int database_id, bool first_time)
      : ConnectedDevice(lister, unique_id, manager, database_id),
        watcher_(new BackgroundThreadImplementation<LibraryWatcher, LibraryWatcher>(this))
{
  // Create the library watcher
  watcher_->Start(true);
  watcher_->Worker()->set_device_name(manager->data(manager->index(
      manager->FindDeviceById(unique_id)), DeviceManager::Role_FriendlyName).toString());
  watcher_->Worker()->set_backend(backend_);
  watcher_->Worker()->set_task_manager(manager_->task_manager());

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

  if (first_time)
    backend_->AddDirectory(mount_point);
  else {
    // This is a bit of a hack.  The device might not be mounted at the same
    // path each time, so if it's different we have to munge all the paths in
    // the database to fix it.  This can be done entirely in sqlite so it's
    // relatively fast...

    // Get the directory it was mounted at last time.  Devices only have one
    // directory (the root).
    Directory dir = backend_->GetAllDirectories()[0];
    if (dir.path != mount_point) {
      // The directory is different, commence the munging.
      qDebug() << "Changing path from" << dir.path << "to" << mount_point;
      backend_->ChangeDirPath(dir.id, mount_point);
    }

    // Load the directory properly now, this signals the watcher as well.
    backend_->LoadDirectoriesAsync();
  }

  model_->Init();
}

FilesystemDevice::~FilesystemDevice() {
  qDebug() << __PRETTY_FUNCTION__;
}
