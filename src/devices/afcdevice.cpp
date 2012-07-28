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

#include "afcdevice.h"
#include "afcfile.h"
#include "afctransfer.h"
#include "devicemanager.h"
#include "gpodloader.h"
#include "imobiledeviceconnection.h"
#include "core/application.h"
#include "core/utilities.h"

#include <QThread>

AfcDevice::AfcDevice(
    const QUrl& url, DeviceLister* lister, const QString& unique_id,
    DeviceManager* manager, Application* app, int database_id, bool first_time)
      : GPodDevice(url, lister, unique_id, manager, app, database_id, first_time),
        transfer_(NULL)
{
}

AfcDevice::~AfcDevice() {
  Utilities::RemoveRecursive(local_path_);
}

void AfcDevice::Init() {
  // Make a new temporary directory for the iTunesDB.  We copy it off the iPod
  // so that libgpod can have a local directory to use.
  local_path_ = Utilities::MakeTempDir();
  InitBackendDirectory(local_path_, first_time_, false);
  model_->Init();

  transfer_ = new AfcTransfer(url_.host(), local_path_, app_->task_manager(),
                              shared_from_this());
  transfer_->moveToThread(loader_thread_);

  connect(transfer_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(transfer_, SIGNAL(CopyFinished(bool)), SLOT(CopyFinished(bool)));
  connect(loader_thread_, SIGNAL(started()), transfer_, SLOT(CopyFromDevice()));
  loader_thread_->start();
}

void AfcDevice::CopyFinished(bool success) {
  transfer_->deleteLater();
  transfer_ = NULL;

  if (!success) {
    app_->AddError(tr("An error occurred copying the iTunes database from the device"));
    return;
  }

  // Now load the songs from the local database
  loader_ = new GPodLoader(local_path_, app_->task_manager(), backend_,
                           shared_from_this());
  loader_->set_music_path_prefix("afc://" + url_.host());
  loader_->set_song_type(Song::Type_Stream);
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished(Itdb_iTunesDB*)), SLOT(LoadFinished(Itdb_iTunesDB*)));
  QMetaObject::invokeMethod(loader_, "LoadDatabase");
}

bool AfcDevice::StartCopy(QList<Song::FileType>* supported_types) {
  GPodDevice::StartCopy(supported_types);
  connection_.reset(new iMobileDeviceConnection(url_.host()));

  return true;
}

bool AfcDevice::CopyToStorage(const CopyJob& job) {
  Q_ASSERT(db_);

  Itdb_Track* track = AddTrackToITunesDb(job.metadata_);

  // Get an unused filename on the device
  QString dest = connection_->GetUnusedFilename(db_, job.metadata_);
  if (dest.isEmpty()) {
    itdb_track_remove(track);
    return false;
  }

  // Copy the file
  {
    QFile source_file(job.source_);
    AfcFile dest_file(connection_.get(), dest);
    if (!Utilities::Copy(&source_file, &dest_file))
      return false;
  }

  track->transferred = 1;

  // Set the filetype_marker
  QString suffix = dest.section('.', -1, -1).toUpper();
  track->filetype_marker = 0;
  for (int i=0 ; i<4 ; ++i) {
    track->filetype_marker = track->filetype_marker << 8;
    if (i >= suffix.length())
      track->filetype_marker |= ' ';
    else
      track->filetype_marker |= suffix[i].toAscii();
  }

  // Set the filename
  track->ipod_path = strdup(dest.toUtf8().constData());
  itdb_filename_fs2ipod(track->ipod_path);

  AddTrackToModel(track, "afc://" + url_.host());

  // Remove the original if it was requested
  if (job.remove_original_) {
    QFile::remove(job.source_);
  }

  return true;
}

void AfcDevice::FinishCopy(bool success) {
  // Temporarily unset the GUID so libgpod doesn't lock the device for syncing
  itdb_device_set_sysinfo(db_->device, "FirewireGuid", NULL);

  GPodDevice::FinishCopy(success);

  // Close the connection to the device
  connection_.reset();
}

void AfcDevice::FinaliseDatabase() {
  // Set the GUID again to lock the device for syncing
  itdb_device_set_sysinfo(db_->device, "FirewireGuid", url_.host().toUtf8().constData());

  // Copy the files back to the iPod
  // No need to start another thread since we're already in the organiser thread
  AfcTransfer transfer(url_.host(), local_path_, NULL, shared_from_this());

  itdb_start_sync(db_);
  bool success = transfer.CopyToDevice(connection_.get());
  itdb_stop_sync(db_);

  if (!success) {
    app_->AddError(tr("An error occurred copying the iTunes database onto the device"));
    return;
  }
}

bool AfcDevice::DeleteFromStorage(const DeleteJob& job) {
  const QString path = job.metadata_.url().toLocalFile();

  if (!RemoveTrackFromITunesDb(path))
    return false;

  // Remove the file
  if (afc_remove_path(connection_->afc(), path.toUtf8().constData()) != AFC_E_SUCCESS)
    return false;

  // Remove it from our library model
  songs_to_remove_ << job.metadata_;

  return true;
}
