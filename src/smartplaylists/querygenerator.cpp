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

#include "querygenerator.h"
#include "library/librarybackend.h"
#include "library/libraryplaylistitem.h"

#include <QtDebug>

namespace smart_playlists {

QueryGenerator::QueryGenerator()
  : dynamic_(false)
{
}

void QueryGenerator::Load(const Search& search) {
  search_ = search;
  dynamic_ = false;
}

void QueryGenerator::Load(const QByteArray& data) {
  QDataStream s(data);
  s >> search_;
  s >> dynamic_;
}

QByteArray QueryGenerator::Save() const {
  QByteArray ret;
  QDataStream s(&ret, QIODevice::WriteOnly);
  s << search_;
  s << dynamic_;

  return ret;
}

PlaylistItemList QueryGenerator::Generate() {
  previous_ids_.clear();
  return GenerateMore(0);
}

PlaylistItemList QueryGenerator::GenerateMore(int count) {
  Search search_copy = search_;
  search_copy.id_not_in_ = previous_ids_;
  if (count) {
    search_copy.limit_ = count;
  } else if (dynamic_) {
    search_copy.limit_ = kDynamicFuture;
  }

  SongList songs = backend_->FindSongs(search_copy);
  PlaylistItemList items;
  foreach (const Song& song, songs) {
    items << PlaylistItemPtr(new LibraryPlaylistItem(song));
    previous_ids_ << song.id();

    if (previous_ids_.count() > kDynamicFuture + kDynamicHistory)
      previous_ids_.removeFirst();
  }
  return items;
}

} // namespace
