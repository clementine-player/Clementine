/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "twitterartistinfo.h"

#include <QXmlStreamWriter>

#include <echonest/Artist.h>

#include <qjson/parser.h>

#include "core/closure.h"
#include "songinfotextview.h"

const char* TwitterArtistInfo::kTwitterBucket = "id:twitter";
const char* TwitterArtistInfo::kTwitterTimelineUrl =
    "https://api.twitter.com/1/statuses/user_timeline.json?"
    "include_rts=true&count=10&include_entities=true&screen_name=%1";

TwitterArtistInfo::TwitterArtistInfo()
    : network_(this) {
}

void TwitterArtistInfo::FetchInfo(int id, const Song& metadata) {
  Echonest::Artist::SearchParams params;
  params.push_back(qMakePair(Echonest::Artist::Name, QVariant(metadata.artist())));
  params.push_back(qMakePair(Echonest::Artist::IdSpace, QVariant(kTwitterBucket)));

  QNetworkReply* reply = Echonest::Artist::search(params);
  NewClosure(reply, SIGNAL(finished()), this, SLOT(ArtistSearchFinished(QNetworkReply*, int)), reply, id);
}

void TwitterArtistInfo::ArtistSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();
  try {
    Echonest::Artists artists = Echonest::Artist::parseSearch(reply);
    if (artists.isEmpty()) {
      qLog(Debug) << "Failed to find artist in echonest";
      return;
    }

    const Echonest::Artist& artist = artists[0];
    QString twitter_id;
    foreach (const Echonest::ForeignId& id, artist.foreignIds()) {
      if (id.catalog == "twitter") {
        twitter_id = id.foreign_id;
        break;
      }
    }

    if (twitter_id.isEmpty()) {
      qLog(Debug) << "Failed to fetch Twitter foreign id for artist";
      return;
    }

    QStringList split = twitter_id.split(':');
    if (split.count() != 3) {
      qLog(Debug) << "Funky twitter id:" << twitter_id;
      return;
    }
    FetchUserTimeline(split[2], id);
  } catch (Echonest::ParseError& e) {
    qLog(Error) << "Error parsing echonest reply:" << e.errorType() << e.what();
  }
}

void TwitterArtistInfo::FetchUserTimeline(const QString& twitter_id, int id) {
  QUrl url(QString(kTwitterTimelineUrl).arg(twitter_id));
  QNetworkReply* reply = network_.get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(UserTimelineRequestFinished(QNetworkReply*, QString, int)), reply, twitter_id, id);
}

void TwitterArtistInfo::UserTimelineRequestFinished(QNetworkReply* reply, const QString& twitter_id, int id) {
  QJson::Parser parser;
  bool ok = false;
  QVariant result = parser.parse(reply, &ok);
  if (!ok) {
    qLog(Error) << "Error parsing Twitter reply";
    return;
  }
  CollapsibleInfoPane::Data data;
  data.type_ = CollapsibleInfoPane::Data::Type_Biography;
  data.id_ = "twitter/" + twitter_id;
  data.title_ = QString("Twitter (%1)").arg(twitter_id);

  QString html;
  QXmlStreamWriter writer(&html);
  QVariantList tweets = result.toList();
  foreach (const QVariant& v, tweets) {
    QVariantMap tweet = v.toMap();
    QString text = tweet["text"].toString();

    QVariantMap entities = tweet["entities"].toMap();
    QVariantList urls = entities["urls"].toList();

    writer.writeStartElement("div");

    int offset = 0;
    foreach (const QVariant& u, urls) {
      QVariantMap url = u.toMap();
      QVariantList indices = url["indices"].toList();
      int start_index = indices[0].toInt();
      int end_index = indices[1].toInt();

      writer.writeCharacters(text.mid(offset, start_index));
      offset = end_index;

      writer.writeStartElement("a");
      writer.writeAttribute("href", url["expanded_url"].toString());
      writer.writeCharacters(url["display_url"].toString());
      writer.writeEndElement();
    }
    writer.writeCharacters(text.mid(offset));

    writer.writeEndElement();
  }

  SongInfoTextView* text_view = new SongInfoTextView;
  text_view->SetHtml(html);
  data.contents_ = text_view;

  emit InfoReady(id, data);
  emit Finished(id);
}
