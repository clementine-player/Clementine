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

#include "groovesharksearchplaylisttype.h"
#include "groovesharkservice.h"

const char* GroovesharkSearchPlaylistType::kName = "grooveshark-search";

GroovesharkSearchPlaylistType::GroovesharkSearchPlaylistType(GroovesharkService* service)
  : service_(service) {
}

QIcon GroovesharkSearchPlaylistType::icon(Playlist* playlist) const {
  return QIcon(":providers/grooveshark.png");
}

QString GroovesharkSearchPlaylistType::search_hint_text(Playlist* playlist) const {
  return QObject::tr("Search Grooveshark");
}

QString GroovesharkSearchPlaylistType::empty_playlist_text(Playlist* playlist) const {
  return QObject::tr("Start typing in the search box above to find music on Grooveshark.");
}

bool GroovesharkSearchPlaylistType::has_special_search_behaviour(Playlist* playlist) const {
  return true;
}

void GroovesharkSearchPlaylistType::Search(const QString& text, Playlist* playlist) {
  service_->Search(text, playlist);
}

