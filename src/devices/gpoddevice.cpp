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
        loader_(new GPodLoader(url.path(), manager->task_manager(), backend_)),
        active_copy_db_(NULL)
{
  InitBackendDirectory(url.path(), first_time);
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

void GPodDevice::StartCopy() {
  active_copy_mutex_.lock();

  // Load the iTunes database
  GError* error = NULL;
  active_copy_db_ = itdb_parse(url_.path().toLocal8Bit(), &error);

  // Check for errors
  if (!active_copy_db_) {
    qDebug() << "GPodDevice error:" << error->message;
    emit Error(QString::fromUtf8(error->message));
    g_error_free(error);
  }
}

bool GPodDevice::CopyToStorage(
    const QString& source, const QString&,
    const Song& metadata, bool, bool remove_original)
{
  if (!active_copy_db_)
    return false;

  // Create the track
  Itdb_Track* track = itdb_track_new();
  metadata.ToItdb(track);

  // Add it to the DB and the master playlist
  // The DB takes ownership of the track
  itdb_track_add(active_copy_db_, track, -1);
  Itdb_Playlist* mpl = itdb_playlist_mpl(active_copy_db_);
  itdb_playlist_add_track(mpl, track, -1);

  // Copy the file
  GError* error = NULL;
  itdb_cp_track_to_ipod(track, source.toLocal8Bit().constData(), &error);
  if (error) {
    qDebug() << "GPodDevice error:" << error->message;
    emit Error(QString::fromUtf8(error->message));
    g_error_free(error);

    // Need to remove the track from the db again
    itdb_track_remove(track);
    return false;
  }

  return true;
}

void GPodDevice::FinishCopy() {
  GError* error = NULL;
  itdb_write(active_copy_db_, &error);
  if (error) {
    qDebug() << "GPodDevice error:" << error->message;
    emit Error(QString::fromUtf8(error->message));
    g_error_free(error);
  }

  active_copy_mutex_.unlock();
}

