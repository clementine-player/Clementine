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

#include "jamendoplaylistitem.h"

JamendoPlaylistItem::JamendoPlaylistItem(const QString& type)
    : LibraryPlaylistItem(type) {}

JamendoPlaylistItem::JamendoPlaylistItem(const Song& song)
    : LibraryPlaylistItem("Jamendo") {
  song_ = song;
}

bool JamendoPlaylistItem::InitFromQuery(const SqlRow& query) {
  // Rows from the songs tables come first
  song_.InitFromQuery(query, true, (Song::kColumns.count() + 1) * 2);

  return song_.is_valid();
}

QUrl JamendoPlaylistItem::Url() const { return song_.url(); }
