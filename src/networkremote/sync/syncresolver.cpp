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

#include <algorithm>

#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "playlist/playlistitem.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/querygenerator.h"
#include "smartplaylists/search.h"

SyncResolver::SyncResolver(LibraryBackend* library, const Playlists& playlists,
                           const SmartPlaylists& smart_playlists)
    : library_(library),
      playlists_(playlists),
      smart_playlists_(smart_playlists) {}

SyncResolver::Result SyncResolver::Resolve(const SyncSourceId& source,
                                           bool has_seed, qint64 seed) const {
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

    case SyncSourceId::Kind::SmartPlaylist: {
      if (!smart_playlists_.generator) {
        result.state = State::Unsupported;
        return result;
      }
      // Made afresh each time, so a rolling one such as songs added in the
      // last 30 days moves on.
      smart_playlists::GeneratorPtr generator =
          smart_playlists_.generator(source.parts()[0], source.parts()[1]);
      if (!generator) return result;
      auto* query_gen =
          dynamic_cast<smart_playlists::QueryGenerator*>(generator.get());
      if (has_seed && query_gen && library_ &&
          query_gen->search().sort_type_ ==
              smart_playlists::Search::Sort_Random) {
        // For random smart playlists with a seed, sort deterministically by
        // song ID and run SplitMix64 Fisher-Yates before slicing limit so
        // resumed syncs get a reproducible track subset.
        smart_playlists::Search search = query_gen->search();
        const int limit = search.limit_;
        search.sort_type_ = smart_playlists::Search::Sort_FieldAsc;
        search.sort_field_ = smart_playlists::SearchTerm::Field_Title;
        search.limit_ = -1;
        search.first_item_ = 0;
        SongList songs = library_->FindSongs(search);
        std::sort(songs.begin(), songs.end(),
                  [](const Song& a, const Song& b) { return a.id() < b.id(); });
        quint64 state = static_cast<quint64>(seed);
        auto next_u64 = [&state]() {
          state += 0x9e3779b97f4a7c15ULL;
          quint64 z = state;
          z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
          z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
          return z ^ (z >> 31);
        };
        for (int i = songs.size() - 1; i > 0; --i) {
          const int j =
              static_cast<int>(next_u64() % static_cast<quint64>(i + 1));
          if (i != j) songs.swapItemsAt(i, j);
        }
        if (limit >= 0 && songs.size() > limit) {
          songs = songs.mid(0, limit);
        }
        result.songs = songs;
      } else {
        for (const PlaylistItemPtr& item : generator->Generate()) {
          result.songs << item->Metadata();
        }
      }
      result.state = State::Ready;
      return result;
    }

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
