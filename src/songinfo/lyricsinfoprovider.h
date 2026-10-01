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

#ifndef SONGINFO_LYRICSINFOPROVIDER_H_
#define SONGINFO_LYRICSINFOPROVIDER_H_

#include <QMap>

#include "songinfo/lyricsfetcher.h"
#include "songinfo/songinfoprovider.h"

// The song's lyrics, from LyricsFetcher, as a section of the song info pane.
class LyricsInfoProvider : public SongInfoProvider {
  Q_OBJECT

 public:
  LyricsInfoProvider();

  void FetchInfo(int id, const Song& metadata) override;

 private:
  void FetchFinished(int fetch_id, const Lyrics& lyrics);

  LyricsFetcher* fetcher_;
  // Fetch ids to song info request ids.
  QMap<int, int> requests_;
};

#endif  // SONGINFO_LYRICSINFOPROVIDER_H_
