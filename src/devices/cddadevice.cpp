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

#include <QMutexLocker>

#include "library/librarybackend.h"
#include "library/librarymodel.h"

CddaDevice::CddaDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       Application* app, int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      cd_device_(url),
      cdda_song_loader_(url) {
  connect(&cdda_song_loader_, SIGNAL(SongsLoaded(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(&cdda_song_loader_, SIGNAL(SongsDurationLoaded(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(&cdda_song_loader_, SIGNAL(SongsMetadataLoaded(SongList)), this,
          SLOT(SongsLoaded(SongList)));
  connect(this, SIGNAL(SongsDiscovered(SongList)), model_,
          SLOT(SongsDiscovered(SongList)));
  connect(&cd_device_, SIGNAL(DiscChanged()), SLOT(LoadSongs()));
}

CddaDevice::~CddaDevice() {}

void CddaDevice::Init() { LoadSongs(); }

void CddaDevice::Refresh() {}

void CddaDevice::LoadSongs() {
  song_count_ = 0;  // Reset song count, in case it was already set
  cdda_song_loader_.LoadSongs();
}

void CddaDevice::SongsLoaded(const SongList& songs) {
  model_->Reset();
  emit SongsDiscovered(songs);
  song_count_ = songs.size();
}
