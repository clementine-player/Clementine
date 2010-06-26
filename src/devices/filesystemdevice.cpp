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
#include "library/librarywatcher.h"

#include <QtDebug>

FilesystemDevice::FilesystemDevice(
    const QString& mount_point, DeviceLister* lister, const QString& id,
    DeviceManager* manager)
      : ConnectedDevice(lister, id, manager),
        watcher_(new BackgroundThreadImplementation<LibraryWatcher, LibraryWatcher>(this))
{
  // Create the library watcher
  watcher_->Start(true);
  watcher_->Worker()->set_device_name(lister_->DeviceInfo(
      unique_id_, DeviceLister::Field_FriendlyName).toString());
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

  backend_->AddDirectory(mount_point);
}

FilesystemDevice::~FilesystemDevice() {
  qDebug() << __PRETTY_FUNCTION__;
}
