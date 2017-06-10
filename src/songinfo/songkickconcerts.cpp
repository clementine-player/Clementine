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
#include <QVBoxLayout>
#include <QXmlStreamWriter>

#include <qjson/parser.h>

#include "core/closure.h"
#include "core/logging.h"
#include "songkickconcertwidget.h"
#include "ui/iconloader.h"

namespace {
const char* kSongkickArtistCalendarUrl =
    "https://api.songkick.com/api/3.0/artists/%1/calendar.json";
const char* kSongkickArtistSearchUrl =
    "https://api.songkick.com/api/3.0/search/artists.json";
const char* kSongkickApiKey = "8rgKfy1WU6IlJFfN";
}  // namespace

SongkickConcerts::SongkickConcerts() {
  Geolocator* geolocator = new Geolocator;
  geolocator->Geolocate();
  connect(geolocator, SIGNAL(Finished(Geolocator::LatLng)),
          SLOT(GeolocateFinished(Geolocator::LatLng)));
  connect(geolocator, SIGNAL(Finished(Geolocator::LatLng)), geolocator,
          SLOT(deleteLater()));
}

void SongkickConcerts::FetchInfo(int id, const Song& metadata) {
  if (metadata.artist().isEmpty()) {
    emit Finished(id);
    return;
  }

  QUrl url(kSongkickArtistSearchUrl);
  url.addQueryItem("apikey", kSongkickApiKey);
  url.addQueryItem("query", metadata.artist());

  QNetworkRequest request(url);
  QNetworkReply* reply = network_.get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ArtistSearchFinished(QNetworkReply*, int)), reply, id);
}

void SongkickConcerts::ArtistSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  QJson::Parser parser;

	QString replyString = reply->readAll();
  QVariantMap json = parser.parse(replyString.toUtf8()).toMap();

  QVariantMap results_page = json["resultsPage"].toMap();
  QVariantMap results = results_page["results"].toMap();
  QVariantList artists = results["artist"].toList();

  if (artists.isEmpty()) {
    emit Finished(id);
    return;
  }

  QVariantMap artist = artists.first().toMap();
  QString artist_id = artist["id"].toString();

  FetchSongkickCalendar(artist_id, id);
}

void SongkickConcerts::FetchSongkickCalendar(const QString& artist_id, int id) {
  QUrl url(QString(kSongkickArtistCalendarUrl).arg(artist_id));
  url.addQueryItem("per_page", "5");
  url.addQueryItem("apikey", kSongkickApiKey);
  qLog(Debug) << url;
  QNetworkReply* reply = network_.get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(CalendarRequestFinished(QNetworkReply*, int)), reply, id);
}

void SongkickConcerts::CalendarRequestFinished(QNetworkReply* reply, int id) {
  QJson::Parser parser;
  bool ok = false;
	QString replyString = reply->readAll();
  QVariant result = parser.parse(replyString.toUtf8(), &ok);

  if (!ok) {
    qLog(Error) << "Error parsing Songkick reply";
    emit Finished(id);
    return;
  }

  QVariantMap root = result.toMap();
  QVariantMap results_page = root["resultsPage"].toMap();
  QVariantMap results = results_page["results"].toMap();
  QVariantList events = results["event"].toList();

  if (events.isEmpty()) {
    emit Finished(id);
    return;
  }

  QWidget* container = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout(container);

  for (const QVariant& v : events) {
    QVariantMap event = v.toMap();
    QString display_name = event["displayName"].toString();
    QString start_date = event["start"].toMap()["date"].toString();
    QString city = event["location"].toMap()["city"].toString();
    QString uri = event["uri"].toString();

    // Try to get the lat/lng coordinates of the venue.
    QVariantMap venue = event["venue"].toMap();
    const bool valid_latlng = venue["lng"].isValid() && venue["lat"].isValid();

    if (valid_latlng && latlng_.IsValid()) {
      static const int kFilterDistanceMetres = 250 * 1e3;  // 250km
      Geolocator::LatLng latlng(venue["lat"].toString(),
                                venue["lng"].toString());
      if (latlng_.IsValid() && latlng.IsValid()) {
        int distance_metres = latlng_.Distance(latlng);
        if (distance_metres > kFilterDistanceMetres) {
          qLog(Debug) << "Filtered concert:" << display_name
                      << "as too far away:" << distance_metres;
          continue;
        }
      }
    }

    SongKickConcertWidget* widget = new SongKickConcertWidget(container);
    widget->Init(display_name, uri, start_date, city);

    if (valid_latlng) {
      widget->SetMap(venue["lat"].toString(), venue["lng"].toString(),
                     venue["displayName"].toString());
    }

    layout->addWidget(widget);
  }

  CollapsibleInfoPane::Data data;
  data.type_ = CollapsibleInfoPane::Data::Type_Biography;
  data.id_ = QString("songkick/%1").arg(id);
  data.title_ = tr("Upcoming Concerts");
  data.icon_ = IconLoader::Load("songkick", IconLoader::Provider);
  data.contents_ = container;

  emit InfoReady(id, data);
  emit Finished(id);
}

void SongkickConcerts::GeolocateFinished(Geolocator::LatLng latlng) {
  latlng_ = latlng;
}
