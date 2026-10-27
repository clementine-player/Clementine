/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "syncresolver.h"

#include "library/librarybackend.h"
#include "library/libraryquery.h"

SyncResolver::SyncResolver(LibraryBackend* library, const Playlists& playlists)
    : library_(library), playlists_(playlists) {}

SyncResolver::Result SyncResolver::Resolve(const SyncSourceId& source) const {
  Result result;
  switch (source.kind()) {
    case SyncSourceId::Kind::Song:
    case SyncSourceId::Kind::Album:
    case SyncSourceId::Kind::Artist:
      return Library(source);

    case SyncSourceId::Kind::Playlist:
      if (playlists_.songs && playlists_.songs(source.id(), &result.songs)) {
        result.state = State::Ready;
      }
      return result;

    case SyncSourceId::Kind::SmartPlaylist:
    case SyncSourceId::Kind::InternetNode:
      result.state = State::Unsupported;
      return result;

    case SyncSourceId::Kind::Invalid:
      break;
  }
  return result;
}

SyncResolver::Result SyncResolver::Library(const SyncSourceId& source) const {
  Result result;
  if (source.kind() == SyncSourceId::Kind::Song) {
    const Song song = library_->GetSongById(source.id());
    if (song.is_valid() && !song.is_unavailable()) {
      result.state = State::Ready;
      result.songs << song;
    }
    return result;
  }

  // An album by its album artist, or the artist where there's none, as
  // albums are told apart; an artist by the songs' own artist, as the
  // library tab and the remotes' libraries group them.
  LibraryQuery query;
  if (source.kind() == SyncSourceId::Kind::Album) {
    query.AddWhere("effective_albumartist", source.parts()[0]);
    query.AddWhere("album", source.parts()[1]);
    query.SetOrderBy("disc, track, title");
  } else {
    query.AddWhere("artist", source.parts()[0]);
    query.SetOrderBy("year, album, disc, track, title");
  }
  result.songs = library_->ExecLibraryQuery(&query);
  // An album or artist with no songs left has gone from the library.
  if (!result.songs.isEmpty()) result.state = State::Ready;
  return result;
}
