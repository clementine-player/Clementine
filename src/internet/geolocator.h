#ifndef GEOLOCATOR_H
#define GEOLOCATOR_H

#include <QObject>

#include "core/network.h"

class Geolocator : public QObject {
  Q_OBJECT
 public:
  explicit Geolocator(QObject* parent = 0);

  void Geolocate();

  class LatLng {
   public:
    LatLng();
    LatLng(int lat_e6, int lng_e6);

    int lat_e6() const { return lat_e6_; }
    int lng_e6() const { return lng_e6_; }

    bool IsValid() const;

   private:
    const int lat_e6_;
    const int lng_e6_;
  };

 signals:
  void Finished(LatLng latlng);

 private slots:
  void RequestFinished(QNetworkReply* reply);

 private:
  NetworkAccessManager network_;

  static const char* kUrl;
};

#endif  // GEOLOCATOR_H
