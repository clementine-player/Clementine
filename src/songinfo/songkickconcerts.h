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

#ifndef SONGKICKCONCERTS_H
#define SONGKICKCONCERTS_H

#include "songinfoprovider.h"

#include "core/network.h"
#include "core/override.h"
#include "internet/core/geolocator.h"

class QNetworkReply;
class SongInfoTextView;

class SongkickConcerts : public SongInfoProvider {
  Q_OBJECT

 public:
  SongkickConcerts();
  void FetchInfo(int id, const Song& metadata) OVERRIDE;

 private slots:
  void ArtistSearchFinished(QNetworkReply* reply, int id);
  void CalendarRequestFinished(QNetworkReply* reply, int id);
  void GeolocateFinished(Geolocator::LatLng latlng);

 private:
  void FetchSongkickCalendar(const QString& artist_id, int id);

  NetworkAccessManager network_;
  Geolocator::LatLng latlng_;
};

#endif
