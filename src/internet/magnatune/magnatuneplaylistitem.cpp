/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "magnatuneplaylistitem.h"
#include "internet/core/internetmodel.h"

MagnatunePlaylistItem::MagnatunePlaylistItem(const QString& type)
    : DbPlaylistItem(type) {}

MagnatunePlaylistItem::MagnatunePlaylistItem(const Song& song)
    : DbPlaylistItem("Magnatune", song) {}

bool MagnatunePlaylistItem::InitFromQuery(const SqlRow& query) {
  // Rows from the songs tables come first
  song_.InitFromQuery(query, true, Song::kColumns.count() + 1);

  return song_.is_valid();
}
