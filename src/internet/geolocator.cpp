#include "geolocator.h"

#include <limits>

#include <qjson/parser.h>

#include <QStringList>

#include "core/closure.h"

const char* Geolocator::kUrl = "https://data.clementine-player.org/geolocate";

using std::numeric_limits;

Geolocator::LatLng::LatLng()
  : lat_e6_(numeric_limits<int>::min()),
    lng_e6_(numeric_limits<int>::min()) {
}

Geolocator::LatLng::LatLng(int lat_e6, int lng_e6)
  : lat_e6_(lat_e6),
    lng_e6_(lng_e6) {
}

bool Geolocator::LatLng::IsValid() const {
  return lat_e6_ != numeric_limits<int>::min() &&
         lng_e6_ != numeric_limits<int>::min();
}

Geolocator::Geolocator(QObject* parent)
  : QObject(parent) {
}

void Geolocator::Geolocate() {
  QNetworkRequest req = QNetworkRequest(QUrl(kUrl));
  QNetworkReply* reply = network_.get(req);
  NewClosure(reply, SIGNAL(finished()), this, SLOT(RequestFinished()));
}

void Geolocator::RequestFinished(QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    emit Finished(LatLng());
    return;
  }

  QJson::Parser parser;
  bool ok = false;
  QVariant result = parser.parse(reply, &ok);
  if (!ok) {
    emit Finished(LatLng());
    return;
  }

  QVariantMap map = result.toMap();
  QString latlng = map["latlng"].toString();
  QStringList split = latlng.split(",");
  if (split.length() != 2) {
    emit Finished(LatLng());
    return;
  }

  double lat = split[0].toDouble();
  double lng = split[1].toDouble();

  emit Finished(
      LatLng(static_cast<int>(lat * 1e6),
             static_cast<int>(lng * 1e6)));
}
