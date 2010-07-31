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
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <QFile>
#include <QtDebug>

#include <gpod/itdb.h>

GPodDevice::GPodDevice(
    const QUrl& url, DeviceLister* lister,
    const QString& unique_id, DeviceManager* manager,
    int database_id, bool first_time)
      : ConnectedDevice(url, lister, unique_id, manager, database_id, first_time),
        loader_thread_(new QThread(this)),
        loader_(new GPodLoader(url.path(), manager->task_manager(), backend_)),
        db_(NULL)
{
  InitBackendDirectory(url.path(), first_time);
  model_->Init();

  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished(Itdb_iTunesDB*)), SLOT(LoadFinished(Itdb_iTunesDB*)));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
  loader_thread_->start();
}

GPodDevice::~GPodDevice() {
}

void GPodDevice::LoadFinished(Itdb_iTunesDB* db) {
  QMutexLocker l(&db_mutex_);
  db_ = db;
  db_wait_cond_.wakeAll();

  loader_->deleteLater();
  loader_ = NULL;
}

void GPodDevice::StartCopy() {
  {
    // Wait for the database to be loaded
    QMutexLocker l(&db_mutex_);
    if (!db_)
      db_wait_cond_.wait(&db_mutex_);
  }

  // Ensure only one "organise files" can be active at any one time
  db_busy_.lock();
}

bool GPodDevice::CopyToStorage(
    const QString& source, const QString&,
    const Song& metadata, bool, bool remove_original)
{
  Q_ASSERT(db_);

  // Create the track
  Itdb_Track* track = itdb_track_new();
  metadata.ToItdb(track);

  // Add it to the DB and the master playlist
  // The DB takes ownership of the track
  itdb_track_add(db_, track, -1);
  Itdb_Playlist* mpl = itdb_playlist_mpl(db_);
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

  // Add it to our LibraryModel
  Song metadata_on_device;
  metadata_on_device.InitFromItdb(track);
  metadata_on_device.set_directory_id(1);
  metadata_on_device.set_filename(url_.path() + metadata_on_device.filename());
  songs_to_add_ << metadata_on_device;

  // Remove the original if it was requested
  if (remove_original) {
    QFile::remove(source);
  }

  return true;
}

void GPodDevice::FinishCopy() {
  // Write the itunes database
  GError* error = NULL;
  itdb_write(db_, &error);
  if (error) {
    qDebug() << "GPodDevice error:" << error->message;
    emit Error(QString::fromUtf8(error->message));
    g_error_free(error);
  } else {
    // Update the library model
    backend_->AddOrUpdateSongs(songs_to_add_);
  }

  songs_to_add_.clear();
  db_busy_.unlock();
}

void GPodDevice::StartDelete() {
  StartCopy();
}

bool GPodDevice::DeleteFromStorage(const Song& metadata) {
  Q_ASSERT(db_);

  // Find the track in the itdb, identify it by its filename
  Itdb_Track* track = NULL;
  for (GList* tracks = db_->tracks ; tracks != NULL ; tracks = tracks->next) {
    Itdb_Track* t = static_cast<Itdb_Track*>(tracks->data);

    itdb_filename_ipod2fs(t->ipod_path);
    if (url_.path() + t->ipod_path == metadata.filename()) {
      track = t;
      break;
    }
  }

  if (track == NULL) {
    qWarning() << "Couldn't find song" << metadata.filename() << "in iTunesDB";
    return false;
  }

  // Remove the track from all playlists
  for (GList* playlists = db_->playlists ; playlists != NULL ; playlists = playlists->next) {
    Itdb_Playlist* playlist = static_cast<Itdb_Playlist*>(playlists->data);

    if (itdb_playlist_contains_track(playlist, track)) {
      itdb_playlist_remove_track(playlist, track);
    }
  }

  // Remove the track from the database, this frees the struct too
  itdb_track_remove(track);

  // Remove the file
  QFile::remove(metadata.filename());

  // Remove it from our library model
  backend_->DeleteSongs(SongList() << metadata);

  return true;
}

void GPodDevice::FinishDelete() {
  FinishCopy();
}

