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

#ifndef INTERNET_LASTFM_LASTFMSCROBBLECACHE_H_
#define INTERNET_LASTFM_LASTFMSCROBBLECACHE_H_

#include <QList>
#include <QString>

#include "lastfmtrack.h"

// A persistent, on-disk queue of tracks waiting to be submitted via
// track.scrobble. Scrobbles are cached here first (rather than sent
// immediately) so that plays aren't lost if Clementine is offline or
// Last.fm is unreachable - CacheSong()/Scrobble() in LastFMService add to
// and flush this queue respectively.
class LastFmScrobbleCache {
 public:
  explicit LastFmScrobbleCache(const QString& username);

  // Appends a track and immediately persists the queue to disk.
  void Add(const LastFmTrack& track);

  QList<LastFmTrack> tracks() const { return tracks_; }
  bool isEmpty() const { return tracks_.isEmpty(); }

  // Removes the first `count` tracks (the ones a submission just succeeded
  // for) and persists the queue to disk.
  void RemoveFirst(int count);

 private:
  QString FilePath() const;
  void Load();
  void Save() const;

  QString username_;
  QList<LastFmTrack> tracks_;
};

#endif  // INTERNET_LASTFM_LASTFMSCROBBLECACHE_H_
