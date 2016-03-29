/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "echonestimages.h"

#include <algorithm>
#include <memory>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include <Artist.h>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

namespace {
static const char* kSpotifyBucket = "spotify";
static const char* kSpotifyArtistUrl = "https://api.spotify.com/v1/artists/%1";
}

EchoNestImages::EchoNestImages() : network_(new NetworkAccessManager) {}

EchoNestImages::~EchoNestImages() {}

void EchoNestImages::FetchInfo(int id, const Song& metadata) {
  Echonest::Artist artist;
  artist.setName(metadata.artist());

  // Search for images directly on echonest.
  // This is currently a bit limited as most results are for last.fm urls that
  // no longer work.
  QNetworkReply* reply = artist.fetchImages();
  RegisterReply(reply, id);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestFinished(QNetworkReply*, int, Echonest::Artist)),
             reply, id, artist);

  // Also look up the artist id for the spotify API so we can directly request
  // images from there too.
  Echonest::Artist::SearchParams params;
  params.push_back(
      qMakePair(Echonest::Artist::Name, QVariant(metadata.artist())));
  QNetworkReply* rosetta_reply = Echonest::Artist::search(
      params,
      Echonest::ArtistInformation(Echonest::ArtistInformation::NoInformation,
                                  QStringList() << kSpotifyBucket));
  RegisterReply(rosetta_reply, id);
  NewClosure(rosetta_reply, SIGNAL(finished()), this,
             SLOT(IdsFound(QNetworkReply*, int)), rosetta_reply, id);
}

void EchoNestImages::RequestFinished(QNetworkReply* reply, int id,
                                     Echonest::Artist artist) {
  reply->deleteLater();
  try {
    artist.parseProfile(reply);
  } catch (Echonest::ParseError e) {
    qLog(Warning) << "Error parsing echonest reply:" << e.errorType()
                  << e.what();
  }

  for (const Echonest::ArtistImage& image : artist.images()) {
    // Echonest still sends these broken URLs for last.fm.
    if (image.url().authority() != "userserve-ak.last.fm") {
      emit ImageReady(id, image.url());
    }
  }
}

void EchoNestImages::IdsFound(QNetworkReply* reply, int request_id) {
  reply->deleteLater();
  try {
    Echonest::Artists artists = Echonest::Artist::parseSearch(reply);
    if (artists.isEmpty()) {
      return;
    }
    const Echonest::ForeignIds& foreign_ids = artists.first().foreignIds();
    for (const Echonest::ForeignId& id : foreign_ids) {
      if (id.catalog.contains("spotify")) {
        DoSpotifyImageRequest(id.foreign_id, request_id);
      }
    }
  } catch (Echonest::ParseError e) {
    qLog(Warning) << "Error parsing echonest reply:" << e.errorType()
                  << e.what();
  }
}

void EchoNestImages::DoSpotifyImageRequest(const QString& id, int request_id) {
  QString artist_id = id.split(":").last();
  QUrl url(QString(kSpotifyArtistUrl).arg(artist_id));
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  RegisterReply(reply, request_id);
  NewClosure(reply, SIGNAL(finished()), [this, reply, request_id]() {
    reply->deleteLater();

    QJsonObject result = QJsonDocument::fromJson(reply->readAll()).object();
    QJsonArray images = result["images"].toArray();
    QList<QPair<QUrl, QSize>> image_urls;
    for (const QJsonValue& image : images) {
      QJsonObject image_result = image.toObject();
      image_urls.append(qMakePair(image_result["url"].toVariant().toUrl(),
                                  QSize(image_result["width"].toInt(),
                                        image_result["height"].toInt())));
    }
    // All the images are the same just different sizes; just pick the largest.
    std::sort(image_urls.begin(), image_urls.end(),
              [](const QPair<QUrl, QSize>& a,
                 const QPair<QUrl, QSize>& b) {
      // Sorted by area ascending.
      return (a.second.height() * a.second.width()) <
          (b.second.height() * b.second.width());
    });
    if (!image_urls.isEmpty()) {
      emit ImageReady(request_id, image_urls.last().first);
    }
  });
}

// Keeps track of replies and emits Finished() when all replies associated with
// a request are finished with.
void EchoNestImages::RegisterReply(QNetworkReply* reply, int id) {
  replies_.insert(id, reply);
  NewClosure(reply, SIGNAL(destroyed()), [this, reply, id]() {
    replies_.remove(id, reply);
    if (!replies_.contains(id)) {
      emit Finished(id);
    }
  });
}
