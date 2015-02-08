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

#include "twitterstatuses.h"

#include <echonest/Artist.h>

#include "songinfotextview.h"
#include "core/closure.h"
#include "core/logging.h"

struct TwitterStatuses::Request {
  Request(int id) : id_(id), artist_(new Echonest::Artist) {}

  int id_;
  std::unique_ptr<Echonest::Artist> artist_;
};

TwitterStatuses::TwitterStatuses() {
  twitter_icon_ = QIcon(":/providers/twitter.png");
}

void TwitterStatuses::FetchInfo(int id, const Song& metadata) {
  RequestPtr request(new Request(id));
  request->artist_->setName(metadata.artist());

  QNetworkReply* reply = request->artist_->fetchTwitter();
  qLog(Debug) << reply->request().url();
  NewClosure(reply, SIGNAL(finished()), this, SLOT(RequestFinished(QNetworkReply*)), reply);
  requests_[reply] = request;
}

void TwitterStatuses::RequestFinished(QNetworkReply* reply) {
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
  qLog(Debug) << "Twitter handle: " << request->artist_->twitter();
  
  if (!request->artist_->twitter().trimmed().isEmpty()) {
    CollapsibleInfoPane::Data data;
    data.id_ = "echonest/twitter";
    data.title_ = tr("Twitter updates");
    data.type_ = CollapsibleInfoPane::Data::Type_Twitter;

    data.icon_ = twitter_icon_;

    SongInfoTextView* editor = new SongInfoTextView;
    QString text = "<a href=\"https://twitter.com/" + request->artist_->twitter() + "\">"
                  + tr("Tweets by @%1").arg(request->artist_->twitter()) 
                  + "</a>";
    
    editor->SetHtml(text);
    data.contents_ = editor;

    emit InfoReady(request->id_, data);
  }

  emit Finished(request->id_);
}
