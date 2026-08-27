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

#ifndef INTERNET_LASTFM_LASTFMTRACK_H_
#define INTERNET_LASTFM_LASTFMTRACK_H_

#include <QDateTime>
#include <QString>

// A plain value type carrying just the fields Last.fm's scrobbling API
// (track.scrobble / track.updateNowPlaying / track.love / track.ban) needs.
struct LastFmTrack {
  enum Source {
    // A track played from a normal, seekable local or remote file.
    Player,
    // A track played from an internet radio stream, where we don't know the
    // real track boundaries or duration up front - see Song::ToLastFmTrack.
    NonPersonalisedBroadcast,
  };

  QString artist;
  QString album_artist;
  QString album;
  QString title;
  int track_number = 0;
  int duration_secs = 0;
  QDateTime timestamp;
  Source source = Player;

  bool is_valid() const { return !artist.isEmpty() && !title.isEmpty(); }
};

#endif  // INTERNET_LASTFM_LASTFMTRACK_H_
