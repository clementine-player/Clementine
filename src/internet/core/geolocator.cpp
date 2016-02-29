/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "geolocator.h"

#include <cmath>
#include <limits>

#include <QStringList>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/closure.h"
#include "core/logging.h"

const char* Geolocator::kUrl = "https://data.clementine-player.org/geolocate";

using std::numeric_limits;

Geolocator::LatLng::LatLng()
    : lat_e6_(numeric_limits<int>::min()),
      lng_e6_(numeric_limits<int>::min()) {}

Geolocator::LatLng::LatLng(int lat_e6, int lng_e6)
    : lat_e6_(lat_e6), lng_e6_(lng_e6) {}

Geolocator::LatLng::LatLng(const QString& latlng)
    : lat_e6_(numeric_limits<int>::min()), lng_e6_(numeric_limits<int>::min()) {
  QStringList split = latlng.split(",");
  if (split.length() != 2) {
    return;
  }

  const double lat = split[0].toDouble();
  const double lng = split[1].toDouble();
  lat_e6_ = static_cast<int>(lat * 1e6);
  lng_e6_ = static_cast<int>(lng * 1e6);
}

Geolocator::LatLng::LatLng(const QString& lat, const QString& lng) {
  lat_e6_ = static_cast<int>(lat.toDouble() * 1e6);
  lng_e6_ = static_cast<int>(lng.toDouble() * 1e6);
}

int Geolocator::LatLng::Distance(const Geolocator::LatLng& other) const {
  static const int kEarthRadiusMetres = 6372800;

  double lat_a = lat_e6() / 1000000.0 * (M_PI / 180.0);
  double lng_a = lng_e6() / 1000000.0 * (M_PI / 180.0);

  double lat_b = other.lat_e6() / 1000000.0 * (M_PI / 180.0);
  double lng_b = other.lng_e6() / 1000000.0 * (M_PI / 180.0);

  double delta_longitude = lng_b - lng_a;

  double sines = sin(lat_a) * sin(lat_b);
  double cosines = cos(lat_a) * cos(lat_b) * cos(delta_longitude);
  double central_angle = acos(sines + cosines);
  double distance_metres = kEarthRadiusMetres * central_angle;
  return static_cast<int>(distance_metres);
}

bool Geolocator::LatLng::IsValid() const {
  return lat_e6_ != numeric_limits<int>::min() &&
         lng_e6_ != numeric_limits<int>::min();
}

Geolocator::Geolocator(QObject* parent) : QObject(parent) {}

void Geolocator::Geolocate() {
  QNetworkRequest req = QNetworkRequest(QUrl(kUrl));
  QNetworkReply* reply = network_.get(req);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestFinished(QNetworkReply*)), reply);
}

void Geolocator::RequestFinished(QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    emit Finished(LatLng());
    return;
  }

  QJsonParseError error;
  QJsonDocument json_document = QJsonDocument::fromJson(reply->readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    emit Finished(LatLng());
    return;
  }

  QJsonObject json_object = json_document.object();
  QString latlng = json_object["latlng"].toString();

  LatLng ll(latlng);
  emit Finished(ll);
}

QDebug operator<<(QDebug dbg, const Geolocator::LatLng& ll) {
  dbg.nospace() << "(" << ll.lat_e6() << "," << ll.lng_e6() << ")";
  return dbg.space();
}
