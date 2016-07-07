#include "spotifyimages.h"

#include <algorithm>

#include <qjson/parser.h>

#include <QPair>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

namespace {
static const char* kSpotifySearchUrl = "https://api.spotify.com/v1/search";
static const char* kSpotifyArtistUrl = "https://api.spotify.com/v1/artists/%1";
}  // namespace

namespace {
QString ExtractSpotifyId(const QString& spotify_uri) {
  return spotify_uri.split(':')[2];
}
}  // namespace

SpotifyImages::SpotifyImages() : network_(new NetworkAccessManager) {}

SpotifyImages::~SpotifyImages() {}

void SpotifyImages::FetchInfo(int id, const Song& metadata) {
  if (metadata.artist().isEmpty()) {
    emit Finished(id);
    return;
  }

  // Fetch artist id.
  QUrl search_url(kSpotifySearchUrl);
  search_url.addQueryItem("q", metadata.artist());
  search_url.addQueryItem("type", "artist");
  search_url.addQueryItem("limit", "1");

  qLog(Debug) << "Fetching artist:" << search_url;

  QNetworkRequest request(search_url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply]() {
    reply->deleteLater();
    QJson::Parser parser;
    QVariantMap result = parser.parse(reply).toMap();
    QVariantMap artists = result["artists"].toMap();
    if (artists.isEmpty()) {
      emit Finished(id);
      return;
    }
    QVariantList items = artists["items"].toList();
    if (items.isEmpty()) {
      emit Finished(id);
      return;
    }
    QVariantMap artist = items.first().toMap();
    QString spotify_uri = artist["uri"].toString();

    FetchImagesForArtist(id, ExtractSpotifyId(spotify_uri));
  });
}

void SpotifyImages::FetchImagesForArtist(int id, const QString& spotify_id) {
  QUrl artist_url(QString(kSpotifyArtistUrl).arg(spotify_id));
  qLog(Debug) << "Fetching images for artist:" << artist_url;
  QNetworkRequest request(artist_url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply]() {
    reply->deleteLater();
    QJson::Parser parser;
    QVariantMap result = parser.parse(reply).toMap();
    QVariantList images = result["images"].toList();
    QList<QPair<QUrl, QSize>> image_candidates;
    for (QVariant i : images) {
      QVariantMap image = i.toMap();
      int height = image["height"].toInt();
      int width = image["width"].toInt();
      QUrl url = image["url"].toUrl();
      image_candidates.append(qMakePair(url, QSize(width, height)));
    }
    if (!image_candidates.isEmpty()) {
      QPair<QUrl, QSize> winner =
          *std::max_element(
              image_candidates.begin(), image_candidates.end(),
              [](const QPair<QUrl, QSize>& a, const QPair<QUrl, QSize>& b) {
                return (a.second.height() * a.second.width()) <
                       (b.second.height() * b.second.width());
              });
      emit ImageReady(id, winner.first);
    }
    emit Finished(id);
  });
}
