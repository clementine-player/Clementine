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
#include "wmdmdevice.h"
#include "wmdmloader.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <QThread>

WmdmDevice::WmdmDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, database_id, first_time),
      loader_thread_(new QThread(this)),
      loader_(NULL)
{
}

WmdmDevice::~WmdmDevice() {
}

void WmdmDevice::Init() {
  InitBackendDirectory("/", first_time_, false);
  model_->Init();

  loader_ = new WmdmLoader(manager_->task_manager(), backend_, shared_from_this());
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished()), SLOT(LoadFinished()));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
  loader_thread_->start();
}

void WmdmDevice::LoadFinished() {
  loader_->deleteLater();
  loader_ = NULL;
}

void WmdmDevice::StartCopy() {
  // Ensure only one "organise files" can be active at any one time
  db_busy_.lock();
}

bool WmdmDevice::CopyToStorage(
    const QString& source, const QString&, const Song& metadata,
    bool, bool remove_original)
{
  return false;
}

void WmdmDevice::FinishCopy(bool success) {
  if (success) {
    if (!songs_to_add_.isEmpty())
      backend_->AddOrUpdateSongs(songs_to_add_);
    if (!songs_to_remove_.isEmpty())
      backend_->DeleteSongs(songs_to_remove_);
  }

  songs_to_add_.clear();
  songs_to_remove_.clear();

  db_busy_.unlock();
}

void WmdmDevice::StartDelete() {
  StartCopy();
}

bool WmdmDevice::DeleteFromStorage(const Song& metadata) {
  return false;
}

void WmdmDevice::FinishDelete(bool success) {
  FinishCopy(success);
}
