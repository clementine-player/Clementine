#include "spotifyimages.h"

#include <algorithm>

#include <qjson/parser.h>

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

  QUrl url(kSpotifyImagesUrl);
  url.addQueryItem("artist", metadata.artist());

  QNetworkRequest request(url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply]() {
    reply->deleteLater();
    QJson::Parser parser;
    bool ok = false;
    QVariant result = parser.parse(reply, &ok);
    if (!ok || result.type() != QVariant::List) {
      emit Finished(id);
      return;
    }

    QVariantList results = result.toList();
    QList<QPair<QUrl, QSize>> image_candidates;
    for (const QVariant& v : results) {
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
