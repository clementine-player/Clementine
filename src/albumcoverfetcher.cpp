#include "albumcoverfetcher.h"

#include <QNetworkReply>
#include <QTimer>

#include <lastfm/Artist>
#include <lastfm/XmlQuery>
#include <lastfm/ws.h>

const int AlbumCoverFetcher::kMaxConcurrentRequests = 5;

AlbumCoverFetcher::AlbumCoverFetcher(QObject* parent, QNetworkAccessManager* network)
    : QObject(parent),
      network_(network),
      next_id_(0),
      request_starter_(new QTimer(this))
{
  if (!network_.get()) {
    network_.reset(new QNetworkAccessManager);
  }
  request_starter_->setInterval(1000);
  connect(request_starter_, SIGNAL(timeout()), SLOT(StartRequests()));
}

quint64 AlbumCoverFetcher::FetchAlbumCover(
    const QString& artist_name, const QString& album_name) {
  QueuedRequest request;
  request.album = album_name;
  request.artist = artist_name;
  request.id = next_id_ ++;

  queued_requests_.enqueue(request);

  if (!request_starter_->isActive())
    request_starter_->start();

  if (active_requests_.count() < kMaxConcurrentRequests)
    StartRequests();

  return request.id;
}

void AlbumCoverFetcher::Clear() {
  queued_requests_.clear();
}

void AlbumCoverFetcher::StartRequests() {
  if (queued_requests_.isEmpty()) {
    request_starter_->stop();
    return;
  }

  while (!queued_requests_.isEmpty() &&
         active_requests_.count() < kMaxConcurrentRequests) {
    QueuedRequest request = queued_requests_.dequeue();

    lastfm::Artist artist(request.artist);
    lastfm::Album album(artist, request.album);

    QNetworkReply* reply = album.getInfo();
    connect(reply, SIGNAL(finished()), SLOT(AlbumGetInfoFinished()));
    active_requests_.insert(reply, request.id);
  }
}

void AlbumCoverFetcher::AlbumGetInfoFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  quint64 id = active_requests_.take(reply);

  try {
    lastfm::XmlQuery query(lastfm::ws::parse(reply));

    QUrl image_url(query["album"]["image size=large"].text());
    QNetworkReply* image_reply = network_->get(QNetworkRequest(image_url));
    connect(image_reply, SIGNAL(finished()), SLOT(AlbumCoverFetchFinished()));

    active_requests_[image_reply] = id;
  } catch (std::runtime_error&) {
    emit AlbumCoverFetched(id, QImage());
  }
}

void AlbumCoverFetcher::AlbumCoverFetchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  QImage image;
  image.loadFromData(reply->readAll());

  quint64 id = active_requests_.take(reply);
  emit AlbumCoverFetched(id, image);
}
