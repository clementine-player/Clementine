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

#include "queryplaylistgenerator.h"
#include "library/librarybackend.h"
#include "library/libraryplaylistitem.h"

#include <QSettings>
#include <QtDebug>

QueryPlaylistGenerator::QueryPlaylistGenerator()
{
}

void QueryPlaylistGenerator::Load(const SmartPlaylistSearch& search) {
  search_ = search;
}

void QueryPlaylistGenerator::Load(const QSettings& s) {
  PlaylistGenerator::Load(s);

  QDataStream stream(s.value("search").toByteArray());
  stream >> search_;
}

PlaylistItemList QueryPlaylistGenerator::Generate() {
  SongList songs = backend_->FindSongs(search_);

  PlaylistItemList items;
  foreach (const Song& song, songs) {
    items << PlaylistItemPtr(new LibraryPlaylistItem(song));
  }
  return items;
}
