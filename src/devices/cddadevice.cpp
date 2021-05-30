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

#include "cddadevice.h"

#include "library/librarybackend.h"
#include "library/librarymodel.h"

const int CddaDevice::kDiscChangePollingIntervalMs = 500;

CddaDevice::CddaDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       Application* app, int database_id, bool first_time,
                       bool watch_for_disc_changes)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      cdio_(nullptr),
      disc_changed_timer_(),
      cdda_song_loader_(url) {
  connect(&cdda_song_loader_, SIGNAL(SongsLoaded(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(&cdda_song_loader_, SIGNAL(SongsDurationLoaded(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(&cdda_song_loader_, SIGNAL(SongsMetadataLoaded(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(this, SIGNAL(SongsDiscovered(SongList)), model_,
          SLOT(SongsDiscovered(SongList)));
  //   connect(&cd_device_, SIGNAL(DiscChanged()), SLOT(DiscChangeDetected()));
  cdio_ = cdio_open(url_.path().toLocal8Bit().constData(), DRIVER_DEVICE);
  Q_ASSERT(cdio_);  // todo: error handling?
  connect(&disc_changed_timer_, SIGNAL(timeout()), SLOT(CheckDiscChanged()));
  WatchForDiscChanges(watch_for_disc_changes);
}

CddaDevice::~CddaDevice() {
  Q_ASSERT(cdio_);
  cdio_destroy(cdio_);
}

void CddaDevice::Init() { LoadSongs(); }

void CddaDevice::Refresh() {}

CddaSongLoader* CddaDevice::loader() { return &cdda_song_loader_; }

void CddaDevice::WatchForDiscChanges(bool watch) {
  if (watch && !disc_changed_timer_.isActive())
    disc_changed_timer_.start(CddaDevice::kDiscChangePollingIntervalMs);
  else if (!watch && disc_changed_timer_.isActive())
    disc_changed_timer_.stop();
}

void CddaDevice::LoadSongs() { cdda_song_loader_.LoadSongs(); }

void CddaDevice::SongsLoaded(const SongList& songs) {
  model_->Reset();
  emit SongsDiscovered(songs);
  song_count_ = songs.size();
}

void CddaDevice::CheckDiscChanged() {
  Q_ASSERT(cdio_);
  if (cdio_get_media_changed(cdio_) == 1) {
    emit DiscChanged();
    song_count_ = 0;
    SongList no_songs;
    SongsLoaded(no_songs);
    LoadSongs();
  }
}
