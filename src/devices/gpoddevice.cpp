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

#include "devicemanager.h"
#include "gpoddevice.h"
#include "gpodloader.h"
#include "library/librarymodel.h"

#include <QtDebug>

#include <gpod/itdb.h>

GPodDevice::GPodDevice(
    const QUrl& url, DeviceLister* lister,
    const QString& unique_id, DeviceManager* manager,
    int database_id, bool first_time)
      : ConnectedDevice(url, lister, unique_id, manager, database_id, first_time),
        loader_thread_(new QThread(this)),
        loader_(new GPodLoader(url.toLocalFile(), manager->task_manager(), backend_))
{
  InitBackendDirectory(url.toLocalFile(), first_time);
  model_->Init();

  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
  loader_thread_->start();
  // TODO: loader cleanup
}

GPodDevice::~GPodDevice() {

}

