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
      cdda_song_loader_(url),
      update_in_progress_(false) {
  connect(&cdda_song_loader_, SIGNAL(SongsUpdated(SongList, bool)), this,
          SLOT(SongsLoaded(SongList, bool)));
  connect(&cdda_song_loader_, SIGNAL(DiscChanged()), this, SLOT(DiscChanged()));
  connect(this, SIGNAL(SongsDiscovered(SongList)), model_,
          SLOT(SongsDiscovered(SongList)));
}

CddaDevice::~CddaDevice() {}

void CddaDevice::Init() {
  Q_ASSERT(song_count_ == 0);
  Refresh();
}

void CddaDevice::Refresh() {
  cdda_song_loader_.LoadSongs();  // will not perform costly operations if media
                                  // has not changed since last time, but
                                  // results in call to SongsLoaded again
}

void CddaDevice::SongsLoaded(const SongList& songs,
                             bool further_updates_possible) {
  if (update_in_progress_ && !further_updates_possible) {
    update_in_progress_ = false;
    model_->Reset();
    song_count_ = songs.size();
    emit SongsDiscovered(songs);
  }
}

void CddaDevice::DiscChanged() { update_in_progress_ = true; }
