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

#include "musicbrainzclient.h"

#include <QCoreApplication>
#include <QNetworkReply>
#include <QSet>
#include <QXmlStreamReader>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

const char* MusicBrainzClient::kTrackUrl =
    "https://musicbrainz.org/ws/2/recording/";
const char* MusicBrainzClient::kDiscUrl =
    "https://musicbrainz.org/ws/2/discid/";
const char* MusicBrainzClient::kDateRegex = "^[12]\\d{3}";
const int MusicBrainzClient::kDefaultTimeout = 5000;  // msec

MusicBrainzClient::MusicBrainzClient(QObject* parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      timeouts_(new NetworkTimeouts(kDefaultTimeout, this)) {}

void MusicBrainzClient::Start(int id, const QString& mbid) {
  typedef QPair<QString, QString> Param;

  QList<Param> parameters;
  parameters << Param("inc", "artists+releases+media");

  QUrl url(kTrackUrl + mbid);
  url.setQueryItems(parameters);
  QNetworkRequest req(url);

  QNetworkReply* reply = network_->get(req);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestFinished(QNetworkReply*, int)), reply, id);
  requests_[id] = reply;

  timeouts_->AddReply(reply);
}

void MusicBrainzClient::StartDiscIdRequest(const QString& discid) {
  typedef QPair<QString, QString> Param;

  QList<Param> parameters;
  parameters << Param("inc", "artists+recordings");

  QUrl url(kDiscUrl + discid);
  url.setQueryItems(parameters);
  QNetworkRequest req(url);

  QNetworkReply* reply = network_->get(req);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(DiscIdRequestFinished(QNetworkReply*)), reply);

  timeouts_->AddReply(reply);
}

void MusicBrainzClient::Cancel(int id) { delete requests_.take(id); }

void MusicBrainzClient::CancelAll() {
  qDeleteAll(requests_.values());
  requests_.clear();
}

void MusicBrainzClient::DiscIdRequestFinished(QNetworkReply* reply) {
  reply->deleteLater();

  ResultList ret;
  QString artist;
  QString album;

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() !=
      200) {
    emit Finished(artist, album, ret);
    return;
  }

  // Parse xml result:
  // -get title
  // -get artist
  // -get all the tracks' tags
  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    QXmlStreamReader::TokenType type = reader.readNext();
    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader.name();
      if (name == "title") {
        album = reader.readElementText();
      } else if (name == "artist") {
        ParseArtist(&reader, &artist);
      } else if (name == "track-list") {
        break;
      }
    }
  }

  while (!reader.atEnd()) {
    QXmlStreamReader::TokenType token = reader.readNext();
    if (token == QXmlStreamReader::StartElement &&
        reader.name() == "recording") {
      ResultList tracks = ParseTrack(&reader);
      foreach(const Result & track, tracks) {
        if (!track.title_.isEmpty()) {
          ret << track;
        }
      }
    } else if (token == QXmlStreamReader::EndElement &&
               reader.name() == "track-list") {
      break;
    }
  }

  emit Finished(artist, album, UniqueResults(ret));
}

void MusicBrainzClient::RequestFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();
  requests_.remove(id);
  ResultList ret;

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() !=
      200) {
    emit Finished(id, ret);
    return;
  }

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    if (reader.readNext() == QXmlStreamReader::StartElement &&
        reader.name() == "recording") {
      ResultList tracks = ParseTrack(&reader);
      foreach(const Result & track, tracks) {
        if (!track.title_.isEmpty()) {
          ret << track;
        }
      }
    }
  }

  emit Finished(id, UniqueResults(ret));
}

MusicBrainzClient::ResultList MusicBrainzClient::ParseTrack(
    QXmlStreamReader* reader) {
  Result result;
  QList<Release> releases;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();

      if (name == "title") {
        result.title_ = reader->readElementText();
      } else if (name == "length") {
        result.duration_msec_ = reader->readElementText().toInt();
      } else if (name == "artist") {
        ParseArtist(reader, &result.artist_);
      } else if (name == "release") {
        releases << ParseRelease(reader);
      }
    }

    if (type == QXmlStreamReader::EndElement && reader->name() == "recording") {
      break;
    }
  }

  ResultList ret;
  if (releases.isEmpty()) {
    ret << result;
  } else {
    foreach(const Release & release, releases) {
      ret << release.CopyAndMergeInto(result);
    }
  }
  return ret;
}

void MusicBrainzClient::ParseArtist(QXmlStreamReader* reader, QString* artist) {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement && reader->name() == "name") {
      *artist = reader->readElementText();
    }

    if (type == QXmlStreamReader::EndElement && reader->name() == "artist") {
      return;
    }
  }
}

MusicBrainzClient::Release MusicBrainzClient::ParseRelease(
    QXmlStreamReader* reader) {
  Release ret;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      if (name == "title") {
        ret.album_ = reader->readElementText();
      } else if (name == "date") {
        QRegExp regex(kDateRegex);
        if (regex.indexIn(reader->readElementText()) == 0) {
          ret.year_ = regex.cap(0).toInt();
        }
      } else if (name == "track-list") {
        ret.track_ =
            reader->attributes().value("offset").toString().toInt() + 1;
        Utilities::ConsumeCurrentElement(reader);
      }
    }

    if (type == QXmlStreamReader::EndElement && reader->name() == "release") {
      break;
    }
  }

  return ret;
}

MusicBrainzClient::ResultList MusicBrainzClient::UniqueResults(
    const ResultList& results) {
  ResultList ret = QSet<Result>::fromList(results).toList();
  qSort(ret);
  return ret;
}
