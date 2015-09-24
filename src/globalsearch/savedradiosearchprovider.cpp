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

#include "savedradiosearchprovider.h"
#include "core/mimedata.h"
#include "ui/iconloader.h"

SavedRadioSearchProvider::SavedRadioSearchProvider(SavedRadio* service,
                                                   Application* app,
                                                   QObject* parent)
    : SimpleSearchProvider(app, parent), service_(service) {
  Init(tr("Your radio streams"), "savedradio",
       IconLoader::Load("document-open-remote", IconLoader::base), 
                        MimeDataContainsUrlsOnly);

  set_max_suggestion_count(3);

  connect(service_, SIGNAL(StreamsChanged()), SLOT(MaybeRecreateItems()));

  RecreateItems();
}

void SavedRadioSearchProvider::RecreateItems() {
  QList<Item> items;

  for (const SavedRadio::Stream& stream : service_->Streams()) {
    Item item;
    item.metadata_.set_title(stream.name_);
    item.metadata_.set_url(stream.url_);
    item.keyword_ = stream.name_;
    items << item;
  }

  SetItems(items);
}
