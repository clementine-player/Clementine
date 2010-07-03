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

ConnectedDevice::ConnectedDevice(DeviceLister* lister, const QString& unique_id,
                                 DeviceManager* manager, int database_id)
  : QObject(manager),
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
  model_->Init();
}

ConnectedDevice::~ConnectedDevice() {
  backend_->deleteLater();
}
