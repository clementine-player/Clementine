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

#include <QtConcurrentRun>

#include "playlist.h"
#include "songloaderinserter.h"
#include "core/logging.h"
#include "core/songloader.h"
#include "core/taskmanager.h"

SongLoaderInserter::SongLoaderInserter(TaskManager* task_manager,
                                       LibraryBackendInterface* library,
                                       const Player* player)
    : task_manager_(task_manager),
      destination_(nullptr),
      row_(-1),
      play_now_(true),
      enqueue_(false),
      library_(library),
      player_(player) {}

SongLoaderInserter::~SongLoaderInserter() { qDeleteAll(pending_); }

void SongLoaderInserter::Load(Playlist* destination, int row, bool play_now,
                              bool enqueue, const QList<QUrl>& urls) {
  destination_ = destination;
  row_ = row;
  play_now_ = play_now;
  enqueue_ = enqueue;

  connect(destination, SIGNAL(destroyed()), SLOT(DestinationDestroyed()));
  connect(this, SIGNAL(PreloadFinished()), SLOT(InsertSongs()));
  connect(this, SIGNAL(EffectiveLoadFinished(const SongList&)), destination,
          SLOT(UpdateItems(const SongList&)));

  for (const QUrl& url : urls) {
    SongLoader* loader = new SongLoader(library_, player_, this);

    SongLoader::Result ret = loader->Load(url);

    if (ret == SongLoader::BlockingLoadRequired) {
      pending_.append(loader);
      continue;
    }

    if (ret == SongLoader::Success)
      songs_ << loader->songs();
    else
      emit Error(tr("Error loading %1").arg(url.toString()));
    delete loader;
  }

  if (pending_.isEmpty()) {
    InsertSongs();
    deleteLater();
  } else {
    QtConcurrent::run(this, &SongLoaderInserter::AsyncLoad);
  }
}

// Load audio CD tracks:
// First, we add tracks (without metadata) into the playlist
// In the meantime, MusicBrainz will be queried to get songs' metadata.
// AudioCDTagsLoaded will be called next, and playlist's items will be updated.
void SongLoaderInserter::LoadAudioCD(Playlist* destination, int row,
                                     bool play_now, bool enqueue) {
  destination_ = destination;
  row_ = row;
  play_now_ = play_now;
  enqueue_ = enqueue;

  SongLoader* loader = new SongLoader(library_, player_, this);
  connect(loader, SIGNAL(LoadAudioCDFinished(bool)), SLOT(AudioCDTagsLoaded(bool)));
  qLog(Info) << "Loading audio CD...";
  SongLoader::Result ret = loader->LoadAudioCD();
  if (ret == SongLoader::Error) {
    emit Error(tr("Error while loading audio CD"));
    delete loader;
  } else {
    songs_ = loader->songs();
    InsertSongs();
  }
}

void SongLoaderInserter::DestinationDestroyed() { destination_ = nullptr; }

void SongLoaderInserter::AudioCDTagsLoaded(bool success) {
  SongLoader* loader = qobject_cast<SongLoader*>(sender());
  if (!loader || !destination_) return;

  if (success)
    destination_->UpdateItems(loader->songs());
  else
    qLog(Error) << "Error while getting audio CD metadata from MusicBrainz";
  deleteLater();
}

void SongLoaderInserter::InsertSongs() {
  // Insert songs (that haven't been completelly loaded) to allow user to see
  // and play them while not loaded completely
  if (destination_) {
    destination_->InsertSongsOrLibraryItems(songs_, row_, play_now_, enqueue_);
  }
}

void SongLoaderInserter::AsyncLoad() {

  // First, quick load raw songs.
  int async_progress = 0;
  int async_load_id = task_manager_->StartTask(tr("Loading tracks"));
  task_manager_->SetTaskProgress(async_load_id, async_progress,
                                 pending_.count());
  for (SongLoader* loader : pending_) {
    loader->LoadFilenamesBlocking();
    task_manager_->SetTaskProgress(async_load_id, ++async_progress);
    songs_ << loader->songs();
  }
  task_manager_->SetTaskFinished(async_load_id);
  emit PreloadFinished();

  // Songs are inserted in playlist, now load them completely.
  async_progress = 0;
  async_load_id = task_manager_->StartTask(tr("Loading tracks info"));
  task_manager_->SetTaskProgress(async_load_id, async_progress, songs_.count());
  SongList songs;
  for (SongLoader* loader : pending_) {
    loader->LoadMetadataBlocking();
    songs << loader->songs();
    task_manager_->SetTaskProgress(async_load_id, songs.count());
  }
  task_manager_->SetTaskFinished(async_load_id);

  // Replace the partially-loaded items by the new ones, fully loaded.
  emit EffectiveLoadFinished(songs);

  deleteLater();
}
