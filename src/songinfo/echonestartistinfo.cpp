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
  site_relevance_["wikipedia"] = 100;
  site_relevance_["last.fm"] = 60;
  site_relevance_["lastfm"] = 60;
  site_relevance_["amazon"] = 30;

  site_icons_["amazon"] = QIcon(":/providers/amazon.png");
  site_icons_["aol"] = QIcon(":/providers/aol.png");
  site_icons_["cdbaby"] = QIcon(":/providers/cdbaby.png");
  site_icons_["lastfm"] = QIcon(":/last.fm/as.png");
  site_icons_["last.fm"] = QIcon(":/last.fm/as.png");
  site_icons_["mog"] = QIcon(":/providers/mog.png");
  site_icons_["mtvmusic"] = QIcon(":/providers/mtvmusic.png");
  site_icons_["myspace"] = QIcon(":/providers/myspace.png");
  site_icons_["wikipedia"] = QIcon(":/providers/wikipedia.png");
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
      try {
        request->artist_->parseProfile(reply);
      } catch (Echonest::ParseError e) {
        qWarning() << "Error parsing echonest reply:" << e.errorType() << e.what();
      }

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

  QSet<QString> already_seen;

  foreach (const Echonest::Biography& bio, request->artist_->biographies()) {
    if (already_seen.contains(bio.text()))
      continue;
    already_seen.insert(bio.text());

    CollapsibleInfoPane::Data data;
    data.title_ = tr("Biography from %1").arg(bio.site());
    data.type_ = CollapsibleInfoPane::Data::Type_Biography;

    const QString site = bio.site().toLower();
    if (site_relevance_.contains(site))
      data.relevance_ = site_relevance_[site];
    if (site_icons_.contains(site))
      data.icon_ = site_icons_[site];

    AutoSizedTextEdit* editor = new AutoSizedTextEdit;
    editor->setHtml(bio.text());
    data.contents_ = editor;

    emit InfoReady(request->id_, data);
  }
}
