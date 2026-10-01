/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "songinfo/songinfoview.h"

#include "songinfo/lastfmtrackinfoprovider.h"
#include "songinfo/lyricsinfoprovider.h"

const char* SongInfoView::kSettingsGroup = "SongInfo";

SongInfoView::SongInfoView(QWidget* parent) : SongInfoBase(parent) {
  fetcher_->AddProvider(new LastfmTrackInfoProvider);
  fetcher_->AddProvider(new LyricsInfoProvider);
}

SongInfoView::~SongInfoView() {}

bool SongInfoView::NeedsUpdate(const Song& old_metadata,
                               const Song& new_metadata) const {
  if (new_metadata.title().isEmpty() || new_metadata.artist().isEmpty())
    return false;

  return old_metadata.title() != new_metadata.title() ||
         old_metadata.artist() != new_metadata.artist();
}

void SongInfoView::InfoResultReady(int id,
                                   const CollapsibleInfoPane::Data& data) {
  if (id != current_request_id_) return;

  AddSection(new CollapsibleInfoPane(data, this));
  CollapseSections();
}

void SongInfoView::ResultReady(int id, const SongInfoFetcher::Result& result) {}
