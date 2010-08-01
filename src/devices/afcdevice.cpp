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

#include "afcdevice.h"
#include "afctransfer.h"
#include "devicemanager.h"
#include "gpodloader.h"
#include "core/utilities.h"

#include <QThread>

AfcDevice::AfcDevice(
    const QUrl& url, DeviceLister* lister, const QString& unique_id,
    DeviceManager* manager, int database_id, bool first_time)
      : ConnectedDevice(url, lister, unique_id, manager, database_id, first_time),
        loader_thread_(new QThread(this)),
        transfer_(NULL),
        loader_(NULL),
        db_(NULL)
{
  // Make a new temporary directory for the iTunesDB.  We copy it off the iPod
  // so that libgpod can have a local directory to use.
  local_path_ = Utilities::MakeTempDir();
  InitBackendDirectory(local_path_, first_time);
  model_->Init();

  transfer_ = new AfcTransfer(url.host(), local_path_, manager_->task_manager());
  transfer_->moveToThread(loader_thread_);

  connect(transfer_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(transfer_, SIGNAL(CopyFinished()), SLOT(CopyFinished()));
  connect(loader_thread_, SIGNAL(started()), transfer_, SLOT(CopyFromDevice()));
  loader_thread_->start();
}

AfcDevice::~AfcDevice() {
  Utilities::RemoveRecursive(local_path_);
}

void AfcDevice::CopyFinished() {
  transfer_->deleteLater();
  transfer_ = NULL;

  // Now load the songs from the local database
  loader_ = new GPodLoader(local_path_, manager_->task_manager(), backend_);
  loader_->set_music_path_prefix("afc://" + url_.host());
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished(Itdb_iTunesDB*)), SLOT(LoadFinished(Itdb_iTunesDB*)));
  QMetaObject::invokeMethod(loader_, "LoadDatabase");
}

void AfcDevice::LoadFinished(Itdb_iTunesDB* db) {
  QMutexLocker l(&db_mutex_);
  db_ = db;
  db_wait_cond_.wakeAll();

  loader_->deleteLater();
  loader_ = NULL;
}

bool AfcDevice::CopyToStorage(const QString &source, const QString &destination,
                              const Song &metadata, bool overwrite,
                              bool remove_original) {
  return false;
}

bool AfcDevice::DeleteFromStorage(const Song &metadata) {
  return false;
}
