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

#ifndef INTERNET_CORE_GEOLOCATOR_H_
#define INTERNET_CORE_GEOLOCATOR_H_

#include <QObject>

#include "core/network.h"

class Geolocator : public QObject {
  Q_OBJECT

 public:
  explicit Geolocator(QObject* parent = nullptr);

  void Geolocate();

  class LatLng {
   public:
    LatLng();
    LatLng(int lat_e6, int lng_e6);
    explicit LatLng(const QString& latlng);
    LatLng(const QString& lat, const QString& lng);

    int lat_e6() const { return lat_e6_; }
    int lng_e6() const { return lng_e6_; }

    bool IsValid() const;
    int Distance(const LatLng& other) const;

   private:
    int lat_e6_;
    int lng_e6_;
  };

 signals:
  void Finished(Geolocator::LatLng latlng);

 private slots:
  void RequestFinished(QNetworkReply* reply);

 private:
  NetworkAccessManager network_;

  static const char* kUrl;
};

QDebug operator<<(QDebug dbg, const Geolocator::LatLng& ll);
Q_DECLARE_METATYPE(Geolocator::LatLng);

#endif  // INTERNET_CORE_GEOLOCATOR_H_
