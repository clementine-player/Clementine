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

#include "digitallyimportedsearchprovider.h"
#include "core/logging.h"
#include "internet/digitallyimportedservicebase.h"

DigitallyImportedSearchProvider::DigitallyImportedSearchProvider(
    DigitallyImportedServiceBase* service, Application* app, QObject* parent)
    : SimpleSearchProvider(app, parent), service_(service) {
  Init(service_->name(), service->api_service_name(), service_->icon(),
       ArtIsInSongMetadata | CanGiveSuggestions | CanShowConfig);

  set_safe_words(QStringList() << "sky.fm"
                               << "skyfm"
                               << "di.fm"
                               << "difm"
                               << "digitallyimported");
  set_max_suggestion_count(5);

  connect(service_, SIGNAL(StreamsChanged()), SLOT(MaybeRecreateItems()));

  // Load the channel list on startup only if it doesn't involve going to update
  // info from the server.
  if (!service_->IsChannelListStale()) RecreateItems();
}

void DigitallyImportedSearchProvider::RecreateItems() {
  QList<Item> items;

  DigitallyImportedClient::ChannelList channels = service_->Channels();

  foreach(const DigitallyImportedClient::Channel & channel, channels) {
    Song song;
    service_->SongFromChannel(channel, &song);
    items << Item(song);
  }

  SetItems(items);
}

void DigitallyImportedSearchProvider::ShowConfig() {
  service_->ShowSettingsDialog();
}
