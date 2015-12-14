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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <Artist.h>

#include "core/closure.h"
#include "core/logging.h"
#include "songkickconcertwidget.h"
#include "ui/iconloader.h"

const char* SongkickConcerts::kSongkickArtistBucket = "id:songkick";
const char* SongkickConcerts::kSongkickArtistCalendarUrl =
    "https://api.songkick.com/api/3.0/artists/%1/calendar.json?"
    "per_page=5&"
    "apikey=8rgKfy1WU6IlJFfN";

SongkickConcerts::SongkickConcerts() {
  Geolocator* geolocator = new Geolocator;
  geolocator->Geolocate();
  connect(geolocator, SIGNAL(Finished(Geolocator::LatLng)),
          SLOT(GeolocateFinished(Geolocator::LatLng)));
  NewClosure(geolocator, SIGNAL(Finished(Geolocator::LatLng)), geolocator,
             SLOT(deleteLater()));
}

void SongkickConcerts::FetchInfo(int id, const Song& metadata) {
  Echonest::Artist::SearchParams params;
  params.push_back(
      qMakePair(Echonest::Artist::Name, QVariant(metadata.artist())));
  params.push_back(
      qMakePair(Echonest::Artist::IdSpace, QVariant(kSongkickArtistBucket)));
  qLog(Debug) << "Params:" << params;
  QNetworkReply* reply = Echonest::Artist::search(params);
  qLog(Debug) << reply->request().url();
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ArtistSearchFinished(QNetworkReply*, int)), reply, id);
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
    for (const Echonest::ForeignId& id : foreign_ids) {
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
  }
  catch (Echonest::ParseError& e) {
    qLog(Error) << "Error parsing echonest reply:" << e.errorType() << e.what();
    emit Finished(id);
  }
}

void SongkickConcerts::FetchSongkickCalendar(const QString& artist_id, int id) {
  QUrl url(QString(kSongkickArtistCalendarUrl).arg(artist_id));
  qLog(Debug) << url;
  QNetworkReply* reply = network_.get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(CalendarRequestFinished(QNetworkReply*, int)), reply, id);
}

void SongkickConcerts::CalendarRequestFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  QJsonParseError error;
  QJsonDocument json_document = QJsonDocument::fromJson(reply->readAll(), &error);

  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Error parsing Songkick reply";
    emit Finished(id);
    return;
  }

  QJsonObject json_root = json_document.object();
  QJsonObject json_results_page = json_root["resultsPage"].toObject();
  QJsonObject json_results = json_results_page["results"].toObject();
  QJsonArray json_events = json_results["event"].toArray();

  if (json_events.isEmpty()) {
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

    // Try to get the lat/lng coordinates of the venue.
    QJsonObject json_venue = json_event["venue"].toObject();
    const bool valid_latlng = !json_venue["lng"].isUndefined() && !json_venue["lat"].isUndefined();

    if (valid_latlng && latlng_.IsValid()) {
      static const int kFilterDistanceMetres = 250 * 1e3;  // 250km
      Geolocator::LatLng latlng(json_venue["lat"].toString(),
                                json_venue["lng"].toString());
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
      widget->SetMap(json_venue["lat"].toString(), json_venue["lng"].toString(),
                     json_venue["displayName"].toString());
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
