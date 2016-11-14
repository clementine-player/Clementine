/* This file is part of Clementine.
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2013, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kittenloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlStreamReader>

#include "core/closure.h"
#include "core/network.h"

const char* KittenLoader::kFlickrKittenUrl =
    "https://api.flickr.com/services/rest/"
    "?method=flickr.photos.search"
    "&api_key=808b52887b3cc7fe098abd62f6ed1745"
    "&group_id=99442622@N00"
    "&sort=random"
    "&safe_search=1"
    "&content_type=1";

const char* KittenLoader::kFlickrPhotoUrl =
    "https://farm%1.static.flickr.com/%2/%3_%4_m.jpg";

KittenLoader::KittenLoader(QObject* parent) : AlbumCoverLoader(parent) {}

quint64 KittenLoader::LoadKitten(const AlbumCoverLoaderOptions& options) {
  if (!kitten_urls_.isEmpty()) {
    QUrl url = kitten_urls_.dequeue();
    return AlbumCoverLoader::LoadImageAsync(
        options, QString::null, url.toString(), QString::null, QImage());
  }

  Task task;
  task.options = options;
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
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(KittensRetrieved(QNetworkReply*)), reply);
}

void KittenLoader::KittensRetrieved(QNetworkReply* reply) {
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
