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
  QUrlQuery search_url_query;
  search_url_query.addQueryItem("q", metadata.artist());
  search_url_query.addQueryItem("type", "artist");
  search_url_query.addQueryItem("limit", "1");
  search_url.setQuery(search_url_query);

  qLog(Debug) << "Fetching artist:" << search_url;

  QNetworkRequest request(search_url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply]() {
    reply->deleteLater();
    QJsonDocument json_document = QJsonDocument::fromJson(reply->readAll());
    QJsonObject result = json_document.object();
    QJsonObject artists = result["artists"].toObject();
    if (artists.isEmpty()) {
      emit Finished(id);
      return;
    }
    QJsonArray items = artists["items"].toArray();
    if (items.isEmpty()) {
      emit Finished(id);
      return;
    }
    QJsonObject artist = items.first().toObject();
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
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    QJsonObject result = document.object();
    QJsonArray images = result["images"].toArray();
    QList<QPair<QUrl, QSize>> image_candidates;
    for (const QJsonValue& i : images) {
      QJsonObject image = i.toObject();
      int height = image["height"].toInt();
      int width = image["width"].toInt();
      QUrl url = image["url"].toVariant().toUrl();
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
