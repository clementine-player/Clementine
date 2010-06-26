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

ConnectedDevice::ConnectedDevice(DeviceLister* lister, const QString& id,
                                 DeviceManager* manager)
  : QObject(manager),
    lister_(lister),
    unique_id_(id),
    manager_(manager),
    database_(new BackgroundThreadImplementation<Database, MemoryDatabase>(this)),
    backend_(NULL),
    model_(NULL)
{
  qDebug() << __PRETTY_FUNCTION__;
  // Wait for the database thread to start
  database_->Start(true);

  // Create the backend in the database thread.
  // The backend gets parented to the database.
  backend_ = database_->CreateInThread<LibraryBackend>();
  backend_->Init(database_->Worker(), Library::kSongsTable, Library::kDirsTable,
                 Library::kSubdirsTable, Library::kFtsTable);

  // Create the model
  model_ = new LibraryModel(backend_, this);
}
