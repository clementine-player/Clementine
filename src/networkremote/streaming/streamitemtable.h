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

#ifndef NETWORKREMOTE_STREAMING_STREAMITEMTABLE_H_
#define NETWORKREMOTE_STREAMING_STREAMITEMTABLE_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QMutex>

#include "core/song.h"
#include "engines/playbackrequest.h"
#include "streamplanner.h"

// One track a renderer may fetch from MediaHttpServer.
struct StreamItem {
  int id = 0;
  StreamPlan plan;
  MediaPlaybackRequest req;
  Song song;
};

// The items each renderer may fetch, by session token. Written by
// RemoteEngines on the main thread and read by MediaHttpServer on the
// network remote's thread.
class StreamItemTable {
 public:
  // Replaces what the renderer with |token| may fetch.
  void Set(const QByteArray& token, const QHostAddress& peer,
           const QList<StreamItem>& items);
  void Remove(const QByteArray& token);

  // Finds item |id| for |token|, and the only address allowed to fetch it.
  bool Find(const QByteArray& token, int id, StreamItem* item,
            QHostAddress* peer) const;

 private:
  struct Entry {
    QHostAddress peer;
    QList<StreamItem> items;
  };

  mutable QMutex mutex_;
  QMap<QByteArray, Entry> entries_;
};

#endif  // NETWORKREMOTE_STREAMING_STREAMITEMTABLE_H_
