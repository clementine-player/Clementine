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

#include "artistinfoview.h"

#include "songinfo/artistbiography.h"
#include "songinfo/songinfofetcher.h"
#include "songinfo/songkickconcerts.h"
#include "songinfo/spotifyimages.h"
#include "widgets/prettyimageview.h"

ArtistInfoView::ArtistInfoView(QWidget* parent) : SongInfoBase(parent) {
  fetcher_->AddProvider(new SongkickConcerts);
  fetcher_->AddProvider(new SpotifyImages);
  fetcher_->AddProvider(new ArtistBiography);
}

ArtistInfoView::~ArtistInfoView() {}

bool ArtistInfoView::NeedsUpdate(const Song& old_metadata,
                                 const Song& new_metadata) const {
  if (new_metadata.artist().isEmpty()) return false;

  return old_metadata.artist() != new_metadata.artist();
}

void ArtistInfoView::InfoResultReady(int id,
                                     const CollapsibleInfoPane::Data& data) {
  if (id != current_request_id_) return;

  AddSection(new CollapsibleInfoPane(data, this));
  CollapseSections();
}

void ArtistInfoView::ResultReady(int id,
                                 const SongInfoFetcher::Result& result) {
  if (id != current_request_id_) return;

  if (!result.images_.isEmpty()) {
    // Image view goes at the top
    PrettyImageView* image_view = new PrettyImageView(network_, this);
    AddWidget(image_view);

    for (const QUrl& url : result.images_) {
      image_view->AddImage(url);
    }
  }
  CollapseSections();
}
