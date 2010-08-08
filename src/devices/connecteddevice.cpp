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

#include "connecteddevice.h"
#include "devicemanager.h"
#include "core/database.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <QtDebug>

ConnectedDevice::ConnectedDevice(const QUrl& url, DeviceLister* lister,
                                 const QString& unique_id, DeviceManager* manager,
                                 int database_id, bool first_time)
  : QObject(manager),
    url_(url),
    first_time_(first_time),
    lister_(lister),
    unique_id_(unique_id),
    database_id_(database_id),
    manager_(manager),
    backend_(NULL),
    model_(NULL)
{
  qDebug() << __PRETTY_FUNCTION__;

  // Create the backend in the database thread.
  // The backend gets parented to the database.
  backend_ = manager->database()->CreateInThread<LibraryBackend>();
  backend_->Init(manager->database()->Worker(),
                 QString("device_%1_songs").arg(database_id),
                 QString("device_%1_directories").arg(database_id),
                 QString("device_%1_subdirectories").arg(database_id),
                 QString("device_%1_fts").arg(database_id));

  // Create the model
  model_ = new LibraryModel(backend_, this);
}

ConnectedDevice::~ConnectedDevice() {
  backend_->deleteLater();
}

void ConnectedDevice::InitBackendDirectory(
    const QString& mount_point, bool first_time, bool rewrite_path) {
  if (first_time)
    backend_->AddDirectory(mount_point);
  else {
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
        qDebug() << "Changing path from" << dir.path << "to" << mount_point;
        backend_->ChangeDirPath(dir.id, mount_point);
      }
    }

    // Load the directory properly now
    backend_->LoadDirectoriesAsync();
  }
}

void ConnectedDevice::Eject() {
  manager_->UnmountAsync(manager_->FindDeviceById(unique_id_));
}
