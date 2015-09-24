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

#include "echonestsimilarartists.h"
#include "tagwidget.h"
#include "core/logging.h"
#include "ui/iconloader.h"

#include <echonest/Artist.h>

Q_DECLARE_METATYPE(QVector<QString>);

void EchoNestSimilarArtists::FetchInfo(int id, const Song& metadata) {
  using Echonest::Artist;

  Artist::SearchParams params;
  params << Artist::SearchParamEntry(Artist::Name, metadata.artist());
  params << Artist::SearchParamEntry(Artist::MinHotttnesss, 0.5);

  QNetworkReply* reply = Echonest::Artist::fetchSimilar(params);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
  requests_[reply] = id;
}

void EchoNestSimilarArtists::RequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply || !requests_.contains(reply)) return;
  reply->deleteLater();

  int id = requests_.take(reply);

  Echonest::Artists artists;
  try {
    artists = Echonest::Artist::parseSimilar(reply);
  }
  catch (Echonest::ParseError e) {
    qLog(Warning) << "Error parsing echonest reply:" << e.errorType()
                  << e.what();
  }

  if (!artists.isEmpty()) {
    CollapsibleInfoPane::Data data;
    data.id_ = "echonest/similarartists";
    data.title_ = tr("Similar artists");
    data.type_ = CollapsibleInfoPane::Data::Type_Similar;
    data.icon_ = IconLoader::Load("echonest", IconLoader::provider);

    TagWidget* widget = new TagWidget(TagWidget::Type_Artists);
    data.contents_ = widget;

    widget->SetIcon(IconLoader::Load("x-clementine-artist", IconLoader::base));

    for (const Echonest::Artist& artist : artists) {
      widget->AddTag(artist.name());
      if (widget->count() >= 10) break;
    }

    emit InfoReady(id, data);
  }

  emit Finished(id);
}
