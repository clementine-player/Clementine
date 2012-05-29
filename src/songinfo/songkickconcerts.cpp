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

#include <echonest/Artist.h>
#include "core/closure.h"

const char* SongkickConcerts::kSongkickArtistBucket = "id:songkick";
const char* SongkickConcerts::kSongkickArtistCalendarUrl =
    "http://api.songkick.com/api/3.0/artists/%1/calendar.json?apikey=";

SongkickConcerts::SongkickConcerts() {

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
      return;
    }

    FetchSongkickCalendar(songkick_id, id);
  } catch (Echonest::ParseError& e) {
    qLog(Error) << "Error parsing echonest reply:" << e.errorType() << e.what();
  }
}

void SongkickConcerts::FetchSongkickCalendar(const QString& artist_id, int id) {
  QUrl url(QString(kSongkickArtistCalendarUrl).arg(artist_id));
  qLog(Debug) << "Would send request to:" << url;
}

void SongkickConcerts::CalendarRequestFinished() {

}
