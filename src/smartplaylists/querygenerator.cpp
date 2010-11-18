/* This file is part of Clementine.

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
{
}

void QueryGenerator::Load(const Search& search) {
  search_ = search;
}

void QueryGenerator::Load(const QByteArray& data) {
  QDataStream s(data);
  s >> search_;
}

QByteArray QueryGenerator::Save() const {
  QByteArray ret;
  QDataStream s(&ret, QIODevice::WriteOnly);
  s << search_;

  return ret;
}

PlaylistItemList QueryGenerator::Generate() {
  SongList songs = backend_->FindSongs(search_);

  PlaylistItemList items;
  foreach (const Song& song, songs) {
    items << PlaylistItemPtr(new LibraryPlaylistItem(song));
  }
  return items;
}

} // namespace
