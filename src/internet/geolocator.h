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

#endif  // GEOLOCATOR_H
