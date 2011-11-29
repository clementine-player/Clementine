/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "groovesharkradio.h"

#include "groovesharkservice.h"
#include "core/logging.h"
#include "internet/internetplaylistitem.h"

GroovesharkRadio::GroovesharkRadio()
  : tag_id_(0),
    first_time_(true) {
}

GroovesharkRadio::GroovesharkRadio(GroovesharkService* service, int tag_id)
  : service_(service),
    tag_id_(tag_id),
    first_time_(true) {
}

void GroovesharkRadio::Load(const QByteArray& data) {
}

QByteArray GroovesharkRadio::Save() const {
  return QByteArray();
}

PlaylistItemList GroovesharkRadio::Generate() {
  PlaylistItemList items;
  if (first_time_) {
    Song song = service_->StartAutoplayTag(tag_id_, autoplay_state_);
    PlaylistItemPtr playlist_item = PlaylistItemPtr(new InternetPlaylistItem(service_, song));
    items << playlist_item;
    first_time_ = false;
  }
  Song song = service_->GetAutoplaySong(autoplay_state_);
  PlaylistItemPtr playlist_item = PlaylistItemPtr(new InternetPlaylistItem(service_, song));
  items << playlist_item;
  return items;
}
