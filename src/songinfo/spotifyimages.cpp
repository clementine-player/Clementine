#include "spotifyimages.h"

#include <algorithm>

#include <QPair>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

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

  // Fetch artist id.
  QUrl url(kSpotifyImagesUrl);
  QUrlQuery url_query;
  url_query.addQueryItem("artist", metadata.artist());
  url.setQuery(url_query);

  qLog(Debug) << "Fetching artist:" << url;

  QNetworkRequest request(url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply]() {
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument json_document = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
      emit Finished(id);
      return;
    }

    QJsonArray results = json_document.array();
    QList<QPair<QUrl, QSize>> image_candidates;
    for (const QJsonValue &v : results) {
      QJsonObject image = v.toObject();
      QUrl url = image["url"].toVariant().toUrl();
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
