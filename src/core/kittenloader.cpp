#include "kittenloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlStreamReader>

#include "core/network.h"

const char* KittenLoader::kFlickrKittenUrl =
    "http://api.flickr.com/services/rest/"
    "?method=flickr.photos.search"
    "&api_key=d4dbb3e0fcb0cc0331da7d6a40df9d26"
    "&tags=cat"
    "&sort=random"
    "&safe_search=1"
    "&content_type=1"
    "&auth_token=72157625568449786-b84430cc6d3a8665"
    "&api_sig=66b58c9c4aa61a48d62409bc6d7b1326";

const char* KittenLoader::kFlickrPhotoUrl =
    "http://farm%1.static.flickr.com/%2/%3_%4_m.jpg";

KittenLoader::KittenLoader(QObject* parent)
    : AlbumCoverLoader(parent) {
}

quint64 KittenLoader::LoadImageAsync(
    const QString& art_automatic,
    const QString& art_manual,
    const QString& song_filename,
    const QImage& embedded_image) {
  if (!kitten_urls_.isEmpty()) {
    QUrl url = kitten_urls_.dequeue();
    return AlbumCoverLoader::LoadImageAsync(
        QString::null,
        url.toString(),
        QString::null,
        QImage());
  }

  Task task;
  {
    QMutexLocker l(&mutex_);
    task.id = next_id_++;
    pending_kittens_.enqueue(task);
  }

  metaObject()->invokeMethod(this, "FetchMoreKittens", Qt::QueuedConnection);

  return task.id;
}

void KittenLoader::FetchMoreKittens() {
  QNetworkRequest req = QNetworkRequest(QUrl(kFlickrKittenUrl));
  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(KittensRetrieved()));
}

void KittenLoader::KittensRetrieved() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.tokenType() == QXmlStreamReader::StartElement) {
      if (reader.name() == "photo") {
        QXmlStreamAttributes attrs = reader.attributes();
        QStringRef farm_id = attrs.value("farm");
        QStringRef photo_id = attrs.value("id");
        QStringRef secret = attrs.value("secret");
        QStringRef server = attrs.value("server");
        QString photo_url = QString(kFlickrPhotoUrl)
            .arg(farm_id.toString())
            .arg(server.toString())
            .arg(photo_id.toString())
            .arg(secret.toString());
        kitten_urls_ << QUrl(photo_url);
      }
    }
  }

  QMutexLocker l(&mutex_);
  while (!kitten_urls_.isEmpty() && !pending_kittens_.isEmpty()) {
    Task task = pending_kittens_.dequeue();
    QUrl kitten_url = kitten_urls_.dequeue();
    task.art_manual = kitten_url.toString();
    task.state = State_TryingManual;
    tasks_.enqueue(task);
  }

  if (kitten_urls_.isEmpty()) {
    FetchMoreKittens();
  }

  metaObject()->invokeMethod(this, "ProcessTasks", Qt::QueuedConnection);
}
