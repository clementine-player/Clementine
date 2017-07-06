#include "spotifyimages.h"

#include <algorithm>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

#include <QPair>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

namespace {
static const char* kSpotifyImagesUrl =
    "https://data.clementine-player.org/fetchimages";
}  // namespace

SpotifyImages::SpotifyImages() : network_(new NetworkAccessManager) {}

SpotifyImages::~SpotifyImages() {}

void SpotifyImages::FetchInfo(int id, const Song& metadata) {
  if (metadata.artist().isEmpty()) {
    emit Finished(id);
    return;
  }
  QUrlQuery url_query;
  QUrl url(kSpotifyImagesUrl);
  url_query.addQueryItem("artist", metadata.artist());
  url.setQuery(url_query);
  
  QNetworkRequest request(url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply]() {
    reply->deleteLater();
    QJsonObject json_result = QJsonDocument::fromJson(reply->readAll()).object();
    #if 0
    if (!ok || result.toVariantMap.type() != QVariant::List) {
      emit Finished(id);
      return;
    }
    #endif
    
    QVariantList results = json_result.toVariantMap().values();
    QList<QPair<QUrl, QSize>> image_candidates;
    for (QVariant v : results) {
      QVariantMap image = v.toMap();
      QUrl url = image["url"].toUrl();
      int height = image["height"].toInt();
      int width = image["width"].toInt();
      image_candidates.append(qMakePair(url, QSize(width, height)));
    }
    if (!image_candidates.isEmpty()) {
      QPair<QUrl, QSize> best = *std::max_element(
          image_candidates.begin(), image_candidates.end(),
          [](const QPair<QUrl, QSize>& a, const QPair<QUrl, QSize>& b) {
            return (a.second.height() * a.second.width()) <
                   (b.second.height() * b.second.width());
          });
      emit ImageReady(id, best.first);
    }
    emit Finished(id);
  });
}
