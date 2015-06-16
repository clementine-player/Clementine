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

#include "echonestbiographies.h"

#include <memory>

#include <echonest5/Artist.h>

#include "songinfotextview.h"
#include "core/logging.h"

struct EchoNestBiographies::Request {
  Request(int id) : id_(id), artist_(new Echonest::Artist) {}

  int id_;
  std::unique_ptr<Echonest::Artist> artist_;
};

EchoNestBiographies::EchoNestBiographies() {
  site_relevance_["wikipedia"] = 100;
  site_relevance_["lastfm"] = 60;
  site_relevance_["amazon"] = 30;

  site_icons_["amazon"] = QIcon(":/providers/amazon.png");
  site_icons_["aol"] = QIcon(":/providers/aol.png");
  site_icons_["cdbaby"] = QIcon(":/providers/cdbaby.png");
  site_icons_["lastfm"] = QIcon(":/last.fm/as.png");
  site_icons_["mog"] = QIcon(":/providers/mog.png");
  site_icons_["mtvmusic"] = QIcon(":/providers/mtvmusic.png");
  site_icons_["myspace"] = QIcon(":/providers/myspace.png");
  site_icons_["wikipedia"] = QIcon(":/providers/wikipedia.png");
}

void EchoNestBiographies::FetchInfo(int id, const Song& metadata) {
  std::shared_ptr<Request> request(new Request(id));
  request->artist_->setName(metadata.artist());

  QNetworkReply* reply = request->artist_->fetchBiographies();
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
  requests_[reply] = request;
}

void EchoNestBiographies::RequestFinished() {
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

  QSet<QString> already_seen;

  for (const Echonest::Biography& bio : request->artist_->biographies()) {
    QString canonical_site = bio.site().toLower();
    canonical_site.replace(QRegExp("[^a-z]"), "");

    if (already_seen.contains(canonical_site)) continue;
    already_seen.insert(canonical_site);

    CollapsibleInfoPane::Data data;
    data.id_ = "echonest/bio/" + bio.site();
    data.title_ = tr("Biography from %1").arg(bio.site());
    data.type_ = CollapsibleInfoPane::Data::Type_Biography;

    if (site_relevance_.contains(canonical_site))
      data.relevance_ = site_relevance_[canonical_site];
    if (site_icons_.contains(canonical_site))
      data.icon_ = site_icons_[canonical_site];

    SongInfoTextView* editor = new SongInfoTextView;
    QString text;
    // Add a link to the bio webpage at the top if we have one
    if (!bio.url().isEmpty()) {
      QString bio_url = bio.url().toEncoded();
      if (bio.site() == "facebook") {
        bio_url.replace("graph.facebook.com", "www.facebook.com");
      }
      text += "<p><a href=\"" + bio_url + "\">" +
              tr("Open in your browser") + "</a></p>";
    }

    text += bio.text();
    if (bio.site() == "last.fm") {
      // Echonest lost formatting and it seems there is currently no plans on
      // Echonest side for changing this.
      // But with last.fm, we can guess newlines: "  " corresponds to a newline
      // (this seems to be because on last.fm' website, extra blank is inserted
      // before <br /> tag, and this blank is kept).
      // This is tricky, but this make the display nicer for last.fm
      // biographies.
      text.replace("  ", "<p>");
    }
    editor->SetHtml(text);
    data.contents_ = editor;

    emit InfoReady(request->id_, data);
  }

  emit Finished(request->id_);
}
