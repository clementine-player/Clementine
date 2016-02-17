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

#include <memory>

#include <echonest/Artist.h>

#include "core/logging.h"

struct EchoNestImages::Request {
  Request(int id) : id_(id), artist_(new Echonest::Artist) {}

  int id_;
  std::unique_ptr<Echonest::Artist> artist_;
};

void EchoNestImages::FetchInfo(int id, const Song& metadata) {
  std::shared_ptr<Request> request(new Request(id));
  request->artist_->setName(metadata.artist());

  QNetworkReply* reply = request->artist_->fetchImages();
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
  requests_[reply] = request;
}

void EchoNestImages::RequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply || !requests_.contains(reply)) return;
  reply->deleteLater();

  RequestPtr request = requests_.take(reply);

  try {
    request->artist_->parseProfile(reply);
  }
  catch (Echonest::ParseError e) {
    qLog(Warning) << "Error parsing echonest reply:" << e.errorType()
                  << e.what();
  }

  for (const Echonest::ArtistImage& image : request->artist_->images()) {
    // Echonest still sends these broken URLs for last.fm.
    if (image.url().authority() != "userserve-ak.last.fm") {
      emit ImageReady(request->id_, image.url());
    }
  }

  emit Finished(request->id_);
}
