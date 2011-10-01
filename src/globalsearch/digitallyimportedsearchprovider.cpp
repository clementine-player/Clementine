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
  DigitallyImportedServiceBase* service, QObject* parent)
    : SimpleSearchProvider(parent),
      service_(service)
{
  Init(service_->name(), service->url_scheme(), service_->icon(), false, false);
  icon_ = ScaleAndPad(QImage(service_->icon_path()));

  set_safe_words(QStringList() << "sky.fm" << "skyfm" << "di.fm" << "difm"
                               << "digitallyimported");

  connect(service_, SIGNAL(StreamsChanged()), SLOT(MaybeRecreateItems()));
}

void DigitallyImportedSearchProvider::LoadArtAsync(int id, const Result& result) {
  emit ArtLoaded(id, icon_);
}

void DigitallyImportedSearchProvider::RecreateItems() {
  QList<Item> items;

  DigitallyImportedServiceBase::StreamList streams = service_->Streams();

  foreach (const DigitallyImportedServiceBase::Stream& stream, streams) {
    Song song;
    song.set_title(stream.name_);
    song.set_artist(service_->service_description());
    song.set_url(QUrl(service_->url_scheme() + "://" + stream.key_));

    items << Item(song);
  }

  SetItems(items);
}
