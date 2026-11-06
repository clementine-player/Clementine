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

#include "networkremote/remotesongloader.h"

#include <QtConcurrentRun>

#include "core/closure.h"
#include "core/logging.h"
#include "core/songloader.h"

RemoteSongLoader::RemoteSongLoader(LibraryBackendInterface* library,
                                   const Player* player)
    : library_(library), player_(player) {
  pool_.setMaxThreadCount(1);
}

RemoteSongLoader::~RemoteSongLoader() { pool_.waitForDone(); }

void RemoteSongLoader::Load(const QList<QUrl>& urls, const Accept& accept,
                            QObject* context, const Done& done) {
  (void)QtConcurrent::run(&pool_, [this, urls, accept, context, done]() {
    const SongList songs = LoadBlocking(urls, accept);
    PostTo(context, done, songs);
  });
}

SongList RemoteSongLoader::LoadBlocking(const QList<QUrl>& urls,
                                        const Accept& accept) const {
  SongList songs;
  for (const QUrl& url : urls) {
    SongLoader loader(library_, player_);
    SongLoader::Result result = loader.Load(url);
    if (result == SongLoader::BlockingLoadRequired)
      result = loader.LoadFilenamesBlocking();
    if (result != SongLoader::Success) {
      qLog(Warning) << "Couldn't load" << url << "for a remote";
      continue;
    }
    loader.LoadMetadataBlocking();

    // A copy, since accept can change it.
    for (Song song : loader.songs()) {
      if (accept(&song)) {
        songs << song;
      } else {
        qLog(Warning) << "Refused" << song.url() << "from" << url
                      << "which a remote asked to add";
      }
    }
  }
  return songs;
}
