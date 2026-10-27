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

#ifndef NETWORKREMOTE_SYNC_SYNCRESOLVER_H_
#define NETWORKREMOTE_SYNC_SYNCRESOLVER_H_

#include <functional>

#include "core/song.h"
#include "networkremote/sync/syncsourceid.h"

class LibraryBackend;

// Finds the songs a sync source holds now, in the source's order: what a
// remote's rule asks for, before limits and delivery.
class SyncResolver {
 public:
  enum class State {
    Ready,
    // The source no longer exists: the song, album, artist or playlist is
    // gone.
    Gone,
    // A kind of source this Clementine can't sync yet.
    Unsupported,
  };

  struct Result {
    State state = State::Gone;
    SongList songs;
  };

  // Where playlists come from. NetworkRemote's reads an open playlist as it
  // is now, and a closed one as it was saved.
  struct Playlists {
    // Whether the playlist exists, and its songs if it does.
    std::function<bool(int id, SongList* songs)> songs;
  };

  SyncResolver(LibraryBackend* library, const Playlists& playlists);

  Result Resolve(const SyncSourceId& source) const;

 private:
  Result Library(const SyncSourceId& source) const;

  LibraryBackend* library_;
  Playlists playlists_;
};

#endif  // NETWORKREMOTE_SYNC_SYNCRESOLVER_H_
