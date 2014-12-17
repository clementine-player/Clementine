/* This file is part of Clementine.
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

GroovesharkRadio::GroovesharkRadio(GroovesharkService* service)
    : service_(service), tag_id_(0), use_tag_(false), first_time_(true) {}

GroovesharkRadio::GroovesharkRadio(GroovesharkService* service, int tag_id)
    : service_(service), tag_id_(tag_id), use_tag_(true), first_time_(true) {}

void GroovesharkRadio::Load(const QByteArray& data) {}

QByteArray GroovesharkRadio::Save() const { return QByteArray(); }

PlaylistItemList GroovesharkRadio::Generate() {
  PlaylistItemList items;
  if (first_time_) {
    Song song;
    if (use_tag_) {
      song = service_->StartAutoplayTag(tag_id_, autoplay_state_);
    } else {
      song = service_->StartAutoplay(autoplay_state_);
    }
    // If the song url isn't valid, stop here
    if (!song.is_valid()) {
      return items;
    }
    PlaylistItemPtr playlist_item =
        PlaylistItemPtr(new InternetPlaylistItem(service_, song));
    items << playlist_item;
    first_time_ = false;
  }
  Song song = service_->GetAutoplaySong(autoplay_state_);
  if (!song.is_valid()) {
    return items;
  }
  PlaylistItemPtr playlist_item =
      PlaylistItemPtr(new InternetPlaylistItem(service_, song));
  items << playlist_item;
  return items;
}
