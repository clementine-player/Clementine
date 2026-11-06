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

#ifndef NETWORKREMOTE_REMOTESONGLOADER_H
#define NETWORKREMOTE_REMOTESONGLOADER_H

#include <QList>
#include <QThreadPool>
#include <QUrl>
#include <functional>

#include "core/song.h"

class LibraryBackendInterface;
class Player;
class QObject;

// Loads the songs a network remote client asks to add by URL. A URL can name
// a folder or a playlist file, which Clementine expands into songs, and the
// playlist's entries can name any file. So each song is checked, rather than
// the URL the client sent.
class RemoteSongLoader {
 public:
  // Whether to add a song, which it may change first.
  using Accept = std::function<bool(Song*)>;
  using Done = std::function<void(const SongList&)>;

  RemoteSongLoader(LibraryBackendInterface* library, const Player* player);
  // Waits for loads that have started.
  ~RemoteSongLoader();

  // Loads `urls` away from the calling thread, then calls `done` on
  // `context`'s thread with the songs `accept` takes, in order. Loads finish
  // in the order they're asked for. `context` must outlive this.
  void Load(const QList<QUrl>& urls, const Accept& accept, QObject* context,
            const Done& done);

  // Loads `urls` on this thread, and returns the songs `accept` takes.
  SongList LoadBlocking(const QList<QUrl>& urls, const Accept& accept) const;

 private:
  LibraryBackendInterface* library_;
  const Player* player_;
  // One thread, so that songs are added in the order they're asked for.
  QThreadPool pool_;
};

#endif  // NETWORKREMOTE_REMOTESONGLOADER_H
