/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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
#include "core/logging.h"
#include "core/application.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <QDir>
#include <QFile>
#include <QThread>
#include <QtDebug>

#include <gpod/itdb.h>

GPodDevice::GPodDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       Application* app, int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      loader_thread_(new QThread(this)),
      loader_(nullptr),
      db_(nullptr) {}

void GPodDevice::Init() {
  InitBackendDirectory(url_.path(), first_time_);
  model_->Init();

  loader_ = new GPodLoader(url_.path(), app_->task_manager(), backend_,
                           shared_from_this());
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SLOT(LoaderError(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished(Itdb_iTunesDB*)),
          SLOT(LoadFinished(Itdb_iTunesDB*)));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
  loader_thread_->start();
}

GPodDevice::~GPodDevice() {}

void GPodDevice::LoadFinished(Itdb_iTunesDB* db) {
  QMutexLocker l(&db_mutex_);
  db_ = db;
  db_wait_cond_.wakeAll();

  loader_thread_->quit();
  loader_thread_->wait(1000);
  loader_thread_->deleteLater();
  loader_thread_ = nullptr;

  loader_->deleteLater();
  loader_ = nullptr;
}

bool GPodDevice::StartCopy(QList<Song::FileType>* supported_filetypes) {
  {
    // Wait for the database to be loaded
    QMutexLocker l(&db_mutex_);
    if (!db_) db_wait_cond_.wait(&db_mutex_);
  }

  // Ensure only one "organise files" can be active at any one time
  db_busy_.lock();

  if (supported_filetypes) GetSupportedFiletypes(supported_filetypes);
  return true;
}

Itdb_Track* GPodDevice::AddTrackToITunesDb(const Song& metadata) {
  // Create the track
  Itdb_Track* track = itdb_track_new();
  metadata.ToItdb(track);

  // Add it to the DB and the master playlist
  // The DB takes ownership of the track
  itdb_track_add(db_, track, -1);
  Itdb_Playlist* mpl = itdb_playlist_mpl(db_);
  itdb_playlist_add_track(mpl, track, -1);

  return track;
}

void GPodDevice::AddTrackToModel(Itdb_Track* track, const QString& prefix) {
  // Add it to our LibraryModel
  Song metadata_on_device;
  metadata_on_device.InitFromItdb(track, prefix);
  metadata_on_device.set_directory_id(1);
  songs_to_add_ << metadata_on_device;
}

bool GPodDevice::CopyToStorage(const CopyJob& job) {
  Q_ASSERT(db_);

  Itdb_Track* track = AddTrackToITunesDb(job.metadata_);

  // Copy the file
  GError* error = nullptr;
  itdb_cp_track_to_ipod(
      track, QDir::toNativeSeparators(job.source_).toLocal8Bit().constData(),
      &error);
  if (error) {
    qLog(Error) << "copying failed:" << QString::fromUtf8(error->message);
    app_->AddError(QString::fromUtf8(error->message));
    g_error_free(error);

    // Need to remove the track from the db again
    itdb_track_remove(track);
    return false;
  }

  AddTrackToModel(track, url_.path());

  // Remove the original if it was requested
  if (job.remove_original_) {
    QFile::remove(job.source_);
  }

  return true;
}

void GPodDevice::WriteDatabase(bool success) {
  if (success) {
    // Write the itunes database
    GError* error = nullptr;
    itdb_write(db_, &error);
    if (error) {
      qLog(Error) << "writing database failed:"
                  << QString::fromUtf8(error->message);
      app_->AddError(QString::fromUtf8(error->message));
      g_error_free(error);
    } else {
      FinaliseDatabase();

      // Update the library model
      if (!songs_to_add_.isEmpty()) backend_->AddOrUpdateSongs(songs_to_add_);
      if (!songs_to_remove_.isEmpty()) backend_->DeleteSongs(songs_to_remove_);
    }
  }

  songs_to_add_.clear();
  songs_to_remove_.clear();
  db_busy_.unlock();
}

void GPodDevice::FinishCopy(bool success) {
  WriteDatabase(success);
  ConnectedDevice::FinishCopy(success);
}

void GPodDevice::StartDelete() { StartCopy(nullptr); }

bool GPodDevice::RemoveTrackFromITunesDb(const QString& path,
                                         const QString& relative_to) {
  QString ipod_filename = path;
  if (!relative_to.isEmpty() && path.startsWith(relative_to))
    ipod_filename.remove(
        0, relative_to.length() + (relative_to.endsWith('/') ? -1 : 0));

  ipod_filename.replace('/', ':');

  // Find the track in the itdb, identify it by its filename
  Itdb_Track* track = nullptr;
  for (GList* tracks = db_->tracks; tracks != nullptr; tracks = tracks->next) {
    Itdb_Track* t = static_cast<Itdb_Track*>(tracks->data);

    if (t->ipod_path == ipod_filename) {
      track = t;
      break;
    }
  }

  if (track == nullptr) {
    qLog(Warning) << "Couldn't find song" << path << "in iTunesDB";
    return false;
  }

  // Remove the track from all playlists
  for (GList* playlists = db_->playlists; playlists != nullptr;
       playlists = playlists->next) {
    Itdb_Playlist* playlist = static_cast<Itdb_Playlist*>(playlists->data);

    if (itdb_playlist_contains_track(playlist, track)) {
      itdb_playlist_remove_track(playlist, track);
    }
  }

  // Remove the track from the database, this frees the struct too
  itdb_track_remove(track);

  return true;
}

bool GPodDevice::DeleteFromStorage(const DeleteJob& job) {
  Q_ASSERT(db_);

  if (!RemoveTrackFromITunesDb(job.metadata_.url().toLocalFile(), url_.path()))
    return false;

  // Remove the file
  if (!QFile::remove(job.metadata_.url().toLocalFile())) return false;

  // Remove it from our library model
  songs_to_remove_ << job.metadata_;

  return true;
}

void GPodDevice::FinishDelete(bool success) {
  WriteDatabase(success);
  ConnectedDevice::FinishDelete(success);
}

void GPodDevice::LoaderError(const QString& message) {
  app_->AddError(message);
}

bool GPodDevice::GetSupportedFiletypes(QList<Song::FileType>* ret) {
  *ret << Song::Type_Mp4;
  *ret << Song::Type_Mpeg;
  return true;
}
