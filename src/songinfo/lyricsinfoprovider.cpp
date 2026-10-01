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

#include "lyricsinfoprovider.h"

#include "songinfo/songinfotextview.h"

LyricsInfoProvider::LyricsInfoProvider() : fetcher_(new LyricsFetcher(this)) {
  connect(fetcher_, &LyricsFetcher::Finished, this,
          &LyricsInfoProvider::FetchFinished);
}

void LyricsInfoProvider::FetchInfo(int id, const Song& metadata) {
  requests_[fetcher_->Fetch(metadata)] = id;
}

void LyricsInfoProvider::FetchFinished(int fetch_id, const Lyrics& lyrics) {
  if (!requests_.contains(fetch_id)) return;
  const int id = requests_.take(fetch_id);

  if (!lyrics.IsEmpty()) {
    CollapsibleInfoPane::Data data;
    data.id_ = "lyrics";
    data.title_ = lyrics.title;
    data.type_ = CollapsibleInfoPane::Data::Type_Lyrics;

    SongInfoTextView* view = new SongInfoTextView;
    view->setPlainText(lyrics.instrumental ? tr("Instrumental") : lyrics.plain);
    data.contents_ = view;

    emit InfoReady(id, data);
  }
  emit Finished(id);
}
