/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "somafmsearchprovider.h"
#include "internet/somafmservice.h"

SomaFMSearchProvider::SomaFMSearchProvider(SomaFMServiceBase* service,
                                           Application* app, QObject* parent)
    : SimpleSearchProvider(app, parent), service_(service) {
  Init(service->name(), service->url_scheme(), service->icon(),
       CanGiveSuggestions);
  set_result_limit(3);
  set_max_suggestion_count(3);
  icon_ = ScaleAndPad(
      service->icon().pixmap(service->icon().availableSizes()[0]).toImage());

  connect(service, SIGNAL(StreamsChanged()), SLOT(MaybeRecreateItems()));

  // Load the stream list on startup only if it doesn't involve going to update
  // info from the server.
  if (!service_->IsStreamListStale()) RecreateItems();
}

void SomaFMSearchProvider::LoadArtAsync(int id, const Result& result) {
  emit ArtLoaded(id, icon_);
}

void SomaFMSearchProvider::RecreateItems() {
  QList<Item> items;

  foreach(const SomaFMService::Stream & stream, service_->Streams()) {
    Item item;
    item.metadata_ = stream.ToSong(service_->name());
    item.keyword_ = stream.title_;
    items << item;
  }

  SetItems(items);
}
