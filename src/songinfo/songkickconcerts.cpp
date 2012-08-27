/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "songkickconcerts.h"

#include <QImage>
#include <QXmlStreamWriter>

#include <echonest/Artist.h>

#include <qjson/parser.h>

#include "core/closure.h"
#include "songinfotextview.h"

const char* SongkickConcerts::kSongkickArtistBucket = "id:songkick";
const char* SongkickConcerts::kSongkickArtistCalendarUrl =
    "http://api.songkick.com/api/3.0/artists/%1/calendar.json?"
    "per_page=5&"
    "apikey=8rgKfy1WU6IlJFfN";

SongkickConcerts::SongkickConcerts() {
  Geolocator* geolocator = new Geolocator;
  geolocator->Geolocate();
  connect(geolocator, SIGNAL(Finished(Geolocator::LatLng)), SLOT(GeolocateFinished(Geolocator::LatLng)));
  NewClosure(geolocator, SIGNAL(Finished(Geolocator::LatLng)), geolocator, SLOT(deleteLater()));
}

void SongkickConcerts::FetchInfo(int id, const Song& metadata) {
  Echonest::Artist::SearchParams params;
  params.push_back(qMakePair(Echonest::Artist::Name, QVariant(metadata.artist())));
  params.push_back(qMakePair(Echonest::Artist::IdSpace, QVariant(kSongkickArtistBucket)));
  qLog(Debug) << "Params:" << params;
  QNetworkReply* reply = Echonest::Artist::search(params);
  qLog(Debug) << reply->request().url();
  NewClosure(reply, SIGNAL(finished()), this, SLOT(ArtistSearchFinished(QNetworkReply*, int)), reply, id);
}

void SongkickConcerts::ArtistSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();
  try {
    Echonest::Artists artists = Echonest::Artist::parseSearch(reply);
    if (artists.isEmpty()) {
      qLog(Debug) << "Failed to find artist in echonest";
      emit Finished(id);
      return;
    }

    const Echonest::Artist& artist = artists[0];
    const Echonest::ForeignIds& foreign_ids = artist.foreignIds();
    QString songkick_id;
    foreach (const Echonest::ForeignId& id, foreign_ids) {
      if (id.catalog == "songkick") {
        songkick_id = id.foreign_id;
        break;
      }
    }

    if (songkick_id.isEmpty()) {
      qLog(Debug) << "Failed to fetch songkick foreign id for artist";
      emit Finished(id);
      return;
    }

    QStringList split = songkick_id.split(':');
    if (split.count() != 3) {
      qLog(Error) << "Weird songkick id";
      emit Finished(id);
      return;
    }

    FetchSongkickCalendar(split[2], id);
  } catch (Echonest::ParseError& e) {
    qLog(Error) << "Error parsing echonest reply:" << e.errorType() << e.what();
    emit Finished(id);
  }
}

void SongkickConcerts::FetchSongkickCalendar(const QString& artist_id, int id) {
  QUrl url(QString(kSongkickArtistCalendarUrl).arg(artist_id));
  qLog(Debug) << url;
  QNetworkReply* reply = network_.get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this, SLOT(CalendarRequestFinished(QNetworkReply*, int)), reply, id);
}

void SongkickConcerts::CalendarRequestFinished(QNetworkReply* reply, int id) {
  static const char* kStaticMapUrl =
      "http://maps.googleapis.com/maps/api/staticmap"
      "?key=AIzaSyDDJqmLOeE1mY_EBONhnQmdXbKtasgCtqg"
      "&sensor=false"
      "&size=100x100"
      "&zoom=12"
      "&center=%1,%2"
      "&markers=%1,%2";

  QJson::Parser parser;
  bool ok = false;
  QVariant result = parser.parse(reply, &ok);

  if (!ok) {
    qLog(Error) << "Error parsing Songkick reply";
    emit Finished(id);
    return;
  }

  QString html;
  QXmlStreamWriter writer(&html);
  SongInfoTextView* text_view = new SongInfoTextView;

  QVariantMap root = result.toMap();
  QVariantMap results_page = root["resultsPage"].toMap();
  QVariantMap results = results_page["results"].toMap();
  QVariantList events = results["event"].toList();

  if (events.isEmpty()) {
    emit Finished(id);
    return;
  }

  foreach (const QVariant& v, events) {
    QVariantMap event = v.toMap();
    {
      QString display_name = event["displayName"].toString();
      QVariantMap venue = event["venue"].toMap();
      const bool valid_latlng =
          venue["lng"].isValid() && venue["lat"].isValid();

      if (valid_latlng && latlng_.IsValid()) {
        static const int kFilterDistanceMetres = 250 * 1e3;  // 250km
        Geolocator::LatLng latlng(
            venue["lat"].toString(), venue["lng"].toString());
        if (latlng_.IsValid() && latlng.IsValid()) {
          int distance_metres = latlng_.Distance(latlng);
          if (distance_metres > kFilterDistanceMetres) {
            qLog(Debug) << "Filtered concert:"
                        << display_name
                        << "as too far away:"
                        << distance_metres;
            continue;
          }
        }
      }

      writer.writeStartElement("div");
      {
        writer.writeStartElement("a");
        writer.writeAttribute("href", event["uri"].toString());
        writer.writeCharacters(display_name);
        writer.writeEndElement();
      }
      if (valid_latlng) {
        writer.writeStartElement("img");
        QString maps_url = QString(kStaticMapUrl).arg(
            venue["lat"].toString(),
            venue["lng"].toString());
        writer.writeAttribute("src", maps_url);
        writer.writeEndElement();

        // QTextDocument does not support loading remote images, so we load
        // them here and then inject them into the document later.
        QNetworkRequest request(maps_url);
        QNetworkReply* reply = network_.get(request);
        NewClosure(reply, SIGNAL(finished()), this,
                   SLOT(InjectImage(QNetworkReply*, SongInfoTextView*)),
                   reply, text_view);
      }
      writer.writeEndElement();
    }
  }

  CollapsibleInfoPane::Data data;
  data.type_ = CollapsibleInfoPane::Data::Type_Biography;
  data.id_ = QString("songkick/%1").arg(id);
  data.title_ = tr("Upcoming Concerts");
  data.icon_ = QIcon(":providers/songkick.png");

  text_view->SetHtml(html);
  data.contents_ = text_view;

  emit InfoReady(id, data);
  emit Finished(id);
}

void SongkickConcerts::InjectImage(
    QNetworkReply* reply, SongInfoTextView* text_view) {
  reply->deleteLater();
  QImage image;
  image.load(reply, "png");
  text_view->document()->addResource(
      QTextDocument::ImageResource,
      reply->request().url(),
      QVariant(image));
}

void SongkickConcerts::GeolocateFinished(Geolocator::LatLng latlng) {
  latlng_ = latlng;
}
