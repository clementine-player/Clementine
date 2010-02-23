#include "albumcoverfetcher.h"

#include <QNetworkReply>

#include <lastfm/Artist>
#include <lastfm/XmlQuery>
#include <lastfm/ws.h>

AlbumCoverFetcher::AlbumCoverFetcher(QObject* parent)
    : QObject(parent) {
}

lastfm::Album AlbumCoverFetcher::FetchAlbumCover(
    const QString& artist_name, const QString& album_name) {
  lastfm::Artist artist(artist_name);
  lastfm::Album album(artist, album_name);

  QNetworkReply* reply = album.getInfo();
  connect(reply, SIGNAL(finished()), SLOT(AlbumGetInfoFinished()));
  requests_.insert(reply, album);
  return album;
}

void AlbumCoverFetcher::AlbumGetInfoFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  lastfm::XmlQuery query(lastfm::ws::parse(reply));
  qDebug() << query["album"]["image size=large"].text();

  QUrl image_url(query["album"]["image size=large"].text());
  QNetworkReply* image_reply = network_.get(QNetworkRequest(image_url));
  connect(image_reply, SIGNAL(finished()), SLOT(AlbumCoverFetchFinished()));

  lastfm::Album album = requests_[reply];
  requests_[image_reply] = album;
}

void AlbumCoverFetcher::AlbumCoverFetchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  QImage image;
  image.loadFromData(reply->readAll());

  lastfm::Album album = requests_.take(reply);
  emit AlbumCoverFetched(album, image);
}
