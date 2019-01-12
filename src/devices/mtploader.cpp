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

#include "mtploader.h"

#include <libmtp.h>

#include "connecteddevice.h"
#include "mtpconnection.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"

MtpLoader::MtpLoader(const QUrl& url, TaskManager* task_manager,
                     LibraryBackend* backend,
                     std::shared_ptr<ConnectedDevice> device)
    : QObject(nullptr),
      device_(device),
      url_(url),
      task_manager_(task_manager),
      backend_(backend) {
  original_thread_ = thread();
}

MtpLoader::~MtpLoader() {}

void MtpLoader::LoadDatabase() {
  int task_id = task_manager_->StartTask(tr("Loading MTP device"));
  emit TaskStarted(task_id);

  bool success = TryLoad();

  moveToThread(original_thread_);

  task_manager_->SetTaskFinished(task_id);
  emit LoadFinished(success);
}

bool MtpLoader::TryLoad() {
  MtpConnection dev(url_);
  if (!dev.is_valid()) {
    emit Error(tr("Error connecting MTP device"));
    return false;
  }

  // Load the list of songs on the device
  SongList songs;
  LIBMTP_track_t* tracks =
      LIBMTP_Get_Tracklisting_With_Callback(dev.device(), nullptr, nullptr);
  while (tracks) {
    LIBMTP_track_t* track = tracks;

    Song song;
    song.InitFromMTP(track, url_.host());
    song.set_directory_id(1);
    songs << song;

    tracks = tracks->next;
    LIBMTP_destroy_track_t(track);
  }

  // Need to remove all the existing songs in the database first
  backend_->DeleteSongs(backend_->FindSongsInDirectory(1));

  // Add the songs we've just loaded
  backend_->AddOrUpdateSongs(songs);

  return true;
}
