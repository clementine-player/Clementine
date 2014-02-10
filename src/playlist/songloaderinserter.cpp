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
      async_load_id_(0),
      async_progress_(0),
      library_(library),
      player_(player) {}

SongLoaderInserter::~SongLoaderInserter() {
  qDeleteAll(pending_);
  qDeleteAll(pending_async_);
}

void SongLoaderInserter::Load(Playlist* destination, int row, bool play_now,
                              bool enqueue, const QList<QUrl>& urls) {
  destination_ = destination;
  row_ = row;
  play_now_ = play_now;
  enqueue_ = enqueue;

  connect(destination, SIGNAL(destroyed()), SLOT(DestinationDestroyed()));
  connect(this, SIGNAL(EffectiveLoadFinished(const SongList&)), destination,
          SLOT(UpdateItems(const SongList&)));

  for (const QUrl& url : urls) {
    SongLoader* loader = new SongLoader(library_, player_, this);

    // we're connecting this before we're even sure if this is an async load
    // to avoid race conditions (signal emission before we're listening to it)
    connect(loader, SIGNAL(LoadFinished(bool)),
            SLOT(PendingLoadFinished(bool)));
    SongLoader::Result ret = loader->Load(url);

    if (ret == SongLoader::WillLoadAsync) {
      pending_.insert(loader);
      continue;
    }

    if (ret == SongLoader::Success)
      songs_ << loader->songs();
    else
      emit Error(tr("Error loading %1").arg(url.toString()));
    delete loader;
  }

  if (pending_.isEmpty())
    Finished();
  else {
    async_progress_ = 0;
    async_load_id_ = task_manager_->StartTask(tr("Loading tracks"));
    task_manager_->SetTaskProgress(async_load_id_, async_progress_,
                                   pending_.count());
  }
}

// Load audio CD tracks:
// First, we add tracks (without metadata) into the playlist
// In the meantine, MusicBrainz will be queried to get songs' metadata.
// AudioCDTagsLoaded will be called next, and playlist's items will be updated.
void SongLoaderInserter::LoadAudioCD(Playlist* destination, int row,
                                     bool play_now, bool enqueue) {
  destination_ = destination;
  row_ = row;
  play_now_ = play_now;
  enqueue_ = enqueue;

  SongLoader* loader = new SongLoader(library_, player_, this);
  connect(loader, SIGNAL(LoadFinished(bool)), SLOT(AudioCDTagsLoaded(bool)));
  qLog(Info) << "Loading audio CD...";
  SongLoader::Result ret = loader->LoadAudioCD();
  if (ret == SongLoader::Error) {
    emit Error(tr("Error while loading audio CD"));
    delete loader;
  }
  songs_ = loader->songs();
  PartiallyFinished();
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

void SongLoaderInserter::PendingLoadFinished(bool success) {
  SongLoader* loader = qobject_cast<SongLoader*>(sender());
  if (!loader || !pending_.contains(loader)) return;
  pending_.remove(loader);
  pending_async_.insert(loader);

  if (success)
    songs_ << loader->songs();
  else
    emit Error(tr("Error loading %1").arg(loader->url().toString()));

  task_manager_->SetTaskProgress(async_load_id_, ++async_progress_);
  if (pending_.isEmpty()) {
    task_manager_->SetTaskFinished(async_load_id_);
    async_progress_ = 0;
    async_load_id_ = task_manager_->StartTask(tr("Loading tracks info"));
    task_manager_->SetTaskProgress(async_load_id_, async_progress_,
                                   pending_async_.count());
    PartiallyFinished();
    QtConcurrent::run(this, &SongLoaderInserter::EffectiveLoad);
  }
}

void SongLoaderInserter::PartiallyFinished() {
  // Insert songs (that haven't been completelly loaded) to allow user to see
  // and play them while not loaded completely
  if (destination_) {
    destination_->InsertSongsOrLibraryItems(songs_, row_, play_now_, enqueue_);
  }
}

void SongLoaderInserter::EffectiveLoad() {
  for (SongLoader* loader : pending_async_) {
    loader->EffectiveSongsLoad();
    task_manager_->SetTaskProgress(async_load_id_, ++async_progress_);
    emit EffectiveLoadFinished(loader->songs());
  }
  task_manager_->SetTaskFinished(async_load_id_);

  deleteLater();
}

void SongLoaderInserter::Finished() {
  if (destination_) {
    destination_->InsertSongsOrLibraryItems(songs_, row_, play_now_, enqueue_);
  }

  deleteLater();
}
