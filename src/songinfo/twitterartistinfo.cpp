#include "twitterartistinfo.h"

#include <echonest/Artist.h>

#include <qjson/parser.h>

#include "core/closure.h"
#include "songinfotextview.h"

const char* TwitterArtistInfo::kTwitterBucket = "id:twitter";
const char* TwitterArtistInfo::kTwitterTimelineUrl =
    "https://api.twitter.com/1/statuses/user_timeline.json?include_rts=1&count=10&screen_name=%1";

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
  QVariantList tweets = result.toList();
  foreach (const QVariant& v, tweets) {
    QVariantMap tweet = v.toMap();
    QString text = tweet["text"].toString();
    html += "<div>";
    html += Qt::escape(text);
    html += "</div>";
  }

  SongInfoTextView* text_view = new SongInfoTextView;
  text_view->SetHtml(html);
  data.contents_ = text_view;

  emit InfoReady(id, data);
  emit Finished(id);
}
