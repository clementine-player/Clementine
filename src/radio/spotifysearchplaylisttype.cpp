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

#include "spotifysearchplaylisttype.h"
#include "spotifyservice.h"

const char* SpotifySearchPlaylistType::kName = "spotify-search";

SpotifySearchPlaylistType::SpotifySearchPlaylistType(SpotifyService* service)
  : service_(service) {
}

QIcon SpotifySearchPlaylistType::icon(Playlist* playlist) const {
  return QIcon(":icons/svg/spotify.svg");
}

QString SpotifySearchPlaylistType::search_hint_text(Playlist* playlist) const {
  return QObject::tr("Search Spotify");
}

QString SpotifySearchPlaylistType::empty_playlist_text(Playlist* playlist) const {
  return QObject::tr("Start typing in the search box above to find music on Spotify.");
}

bool SpotifySearchPlaylistType::has_special_search_behaviour(Playlist* playlist) const {
  return true;
}

void SpotifySearchPlaylistType::Search(const QString& text, Playlist* playlist) {
  service_->Search(text, playlist);
}

void SpotifySearchPlaylistType::DidYouMeanClicked(const QString& text, Playlist* playlist) {
  service_->Search(text, playlist, true);
}
