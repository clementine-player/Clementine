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

#include <QUrl>

#include "library/librarybackend.h"
#include "library/librarymodel.h"

const int kDiscChangePollingIntervalMs = 500;

CddaDevice::CddaDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       Application* app, int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      cdio_(nullptr),
      disc_changed_timer_(),
      cdda_song_loader_(url) {
  connect(&cdda_song_loader_, SIGNAL(SongsUpdated(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(&cdda_song_loader_, SIGNAL(Finished()), this,
          SLOT(SongsLoadingFinished()));
  connect(this, SIGNAL(SongsDiscovered(SongList)), model_,
          SLOT(SongsDiscovered(SongList)));
  connect(&disc_changed_timer_, SIGNAL(timeout()), SLOT(CheckDiscChanged()));
}

CddaDevice::~CddaDevice() {
  if (cdio_) {
    cdio_destroy(cdio_);
    cdio_ = nullptr;
  }
}

bool CddaDevice::Init() {
  if (!cdio_) {
    cdio_ = cdio_open(url_.path().toLocal8Bit().constData(), DRIVER_DEVICE);
    if (!cdio_) return false;
    ForceLoadSongs();
    WatchForDiscChanges(true);
  }
  return true;
}

CddaSongLoader* CddaDevice::loader() { return &cdda_song_loader_; }

CdIo_t* CddaDevice::raw_cdio() { return cdio_; }

bool CddaDevice::IsValid() const { return (cdio_ != nullptr); }

void CddaDevice::WatchForDiscChanges(bool watch) {
  if (watch && !disc_changed_timer_.isActive())
    disc_changed_timer_.start(kDiscChangePollingIntervalMs);
  else if (!watch && disc_changed_timer_.isActive())
    disc_changed_timer_.stop();
}

void CddaDevice::ForceLoadSongs() {
  cdda_song_loader_.LoadSongs();
  disc_changed_timer_.stop();
}

void CddaDevice::LoadSongs() {
  SongList songs = cdda_song_loader_.cached_tracks();
  SongsLoaded(songs);
}

void CddaDevice::SongsLoaded(const SongList& songs) {
  model_->Reset();
  song_count_ = songs.size();
  emit SongsDiscovered(songs);
  // When a disc is inserted, cdio_get_media_changed will
  // return true for two times with a bit of delay in between
  // (at least on linux).
  // We clear cdio_get_media_changed after songs are
  // loaded, so we don't potentially re-read the same disc.terminal
  // There's a slight chance that this hides an actual
  // media change, but this should be rare enough to not
  // be a problem in practice and is easily rectified
  // by user cycling the disc once more.
  cdio_get_media_changed(cdio_);
}

void CddaDevice::SongsLoadingFinished() { disc_changed_timer_.start(); }

void CddaDevice::CheckDiscChanged() {
  if (!cdio_) return;  // do nothing if not initialized

  // do nothing if loader is currently reading;
  // we'd just block until it's finished
  if (cdda_song_loader_.IsActive()) return;

  if (cdio_get_media_changed(cdio_) == 1) {
    emit DiscChanged();
    song_count_ = 0;
    SongList no_songs;
    SongsLoaded(no_songs);
    ForceLoadSongs();
  }
}
