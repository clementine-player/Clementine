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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QXmlStreamWriter>

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
  QUrlQuery url_query;
  url_query.addQueryItem("apikey", kSongkickApiKey);
  url_query.addQueryItem("query", metadata.artist());
  url.setQuery(url_query);

  QNetworkRequest request(url);
  QNetworkReply* reply = network_.get(request);
  connect(reply, &QNetworkReply::finished,
          [=] { this->ArtistSearchFinished(reply, id); });
}

void SongkickConcerts::ArtistSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Debug) << "Songkick request error" << reply->errorString();
    emit Finished(id);
    return;
  }

  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Error parsing Songkick artist reply:"
                << error.errorString();
    emit Finished(id);
    return;
  }

  QJsonObject json = document.object();

  QJsonObject results_page = json["resultsPage"].toObject();
  QJsonObject results = results_page["results"].toObject();
  QJsonArray artists = results["artist"].toArray();

  if (artists.isEmpty()) {
    qLog(Debug) << "No artist found in songkick results.";
    emit Finished(id);
    return;
  }

  QJsonObject artist = artists.first().toObject();
  // Calling toString on a value that is not a string type will return an empty
  // string. But QVariant will do the formatting.
  QString artist_id = artist["id"].toVariant().toString();
  FetchSongkickCalendar(artist_id, id);
}

void SongkickConcerts::FetchSongkickCalendar(const QString& artist_id, int id) {
  QUrl url(QString(kSongkickArtistCalendarUrl).arg(artist_id));
  QUrlQuery url_query;
  url_query.addQueryItem("per_page", "5");
  url_query.addQueryItem("apikey", kSongkickApiKey);
  url.setQuery(url_query);
  qLog(Debug) << url;
  QNetworkReply* reply = network_.get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished,
          [=] { this->CalendarRequestFinished(reply, id); });
}

void SongkickConcerts::CalendarRequestFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Debug) << "Songkick request error" << reply->errorString();
    emit Finished(id);
    return;
  }

  QJsonParseError error;
  QJsonDocument json_document =
      QJsonDocument::fromJson(reply->readAll(), &error);

  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Error parsing Songkick calendar reply:"
                << error.errorString();
    emit Finished(id);
    return;
  }

  QJsonObject json_root = json_document.object();
  QJsonObject json_results_page = json_root["resultsPage"].toObject();
  QJsonObject json_results = json_results_page["results"].toObject();
  QJsonArray json_events = json_results["event"].toArray();

  if (json_events.isEmpty()) {
    qLog(Debug) << "No events found in songkick results.";
    emit Finished(id);
    return;
  }

  QWidget* container = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout(container);

  for (const QJsonValue& v : json_events) {
    QJsonObject json_event = v.toObject();
    QString display_name = json_event["displayName"].toString();
    QString start_date = json_event["start"].toObject()["date"].toString();
    QString city = json_event["location"].toObject()["city"].toString();
    QString uri = json_event["uri"].toString();
    QString lat;
    QString lng;

    // Try to get the lat/lng coordinates of the venue.
    QJsonObject json_venue = json_event["venue"].toObject();
    const bool valid_latlng =
        json_venue.contains("lng") && json_venue.contains("lat");

    if (valid_latlng) {
      lat = json_venue["lat"].toVariant().toString();
      lng = json_venue["lng"].toVariant().toString();
      if (latlng_.IsValid()) {
        static const int kFilterDistanceMetres = 250 * 1e3;  // 250km
        Geolocator::LatLng latlng(lat, lng);
        if (latlng_.IsValid() && latlng.IsValid()) {
          int distance_metres = latlng_.Distance(latlng);
          if (distance_metres > kFilterDistanceMetres) {
            qLog(Debug) << "Filtered concert:" << display_name
                        << "as too far away:" << distance_metres;
            continue;
          }
        }
      }
    }

    SongKickConcertWidget* widget = new SongKickConcertWidget(container);
    widget->Init(display_name, uri, start_date, city);

    if (valid_latlng) {
      widget->SetMap(lat, lng, json_venue["displayName"].toString());
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
