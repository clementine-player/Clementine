/* This file is part of Clementine.

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

#include "echonestartistinfo.h"
#include "widgets/autosizedtextedit.h"

#include <echonest/Artist.h>

#include <boost/scoped_ptr.hpp>

struct EchoNestArtistInfo::Request {
  Request(int id) : id_(id), artist_(new Echonest::Artist) {}

  int id_;
  boost::scoped_ptr<Echonest::Artist> artist_;
  QList<QNetworkReply*> pending_replies_;
};

EchoNestArtistInfo::EchoNestArtistInfo(QObject* parent)
  : ArtistInfoProvider(parent)
{
}

void EchoNestArtistInfo::FetchInfo(int id, const QString& artist_name) {
  boost::shared_ptr<Request> request(new Request(id));
  request->artist_->setName(artist_name);

  ConnectReply(request, request->artist_->fetchBiographies(), SLOT(BiographiesFinished()));
  ConnectReply(request, request->artist_->fetchImages(), SLOT(ImagesFinished()));

  requests_ << request;
}

void EchoNestArtistInfo::ConnectReply(
    boost::shared_ptr<Request> request, QNetworkReply* reply, const char* slot) {
  request->pending_replies_ << reply;
  connect(reply, SIGNAL(finished()), slot);
}

EchoNestArtistInfo::RequestPtr EchoNestArtistInfo::ReplyFinished(QNetworkReply* reply) {
  reply->deleteLater();

  foreach (RequestPtr request, requests_) {
    if (request->pending_replies_.contains(reply)) {
      request->artist_->parseProfile(reply);
      request->pending_replies_.removeAll(reply);

      if (request->pending_replies_.isEmpty()) {
        requests_.removeAll(request);
      }

      return request;
    }
  }
  return RequestPtr();
}

void EchoNestArtistInfo::ImagesFinished() {
  RequestPtr request = ReplyFinished(qobject_cast<QNetworkReply*>(sender()));

  foreach (const Echonest::ArtistImage& image, request->artist_->images()) {
    emit ImageReady(request->id_, image.url());
  }
}

void EchoNestArtistInfo::BiographiesFinished() {
  RequestPtr request = ReplyFinished(qobject_cast<QNetworkReply*>(sender()));

  foreach (const Echonest::Biography& bio, request->artist_->biographies()) {
    QTextEdit* editor = new AutoSizedTextEdit;
    editor->setHtml(bio.text());

    emit InfoReady(request->id_, tr("Biography from %1").arg(bio.site()), editor);
  }
}
