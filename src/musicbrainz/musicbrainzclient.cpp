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
#include "core/logging.h"
#include "core/network.h"

#include <QCoreApplication>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QtDebug>

const char* MusicBrainzClient::kTrackUrl = "http://musicbrainz.org/ws/1/track/";
const char* MusicBrainzClient::kDiscUrl = "http://musicbrainz.org/ws/1/release/";
const int MusicBrainzClient::kDefaultTimeout = 5000; // msec

MusicBrainzClient::MusicBrainzClient(QObject* parent)
  : QObject(parent),
    network_(new NetworkAccessManager(this)),
    timeouts_(new NetworkTimeouts(kDefaultTimeout, this))
{
}

void MusicBrainzClient::Start(int id, const QString& puid) {
  typedef QPair<QString, QString> Param;

  QList<Param> parameters;
  parameters << Param("type", "xml")
             << Param("puid", puid);

  QUrl url(kTrackUrl);
  url.setQueryItems(parameters);
  QNetworkRequest req(url);

  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
  requests_[reply] = id;

  timeouts_->AddReply(reply);
}

void MusicBrainzClient::StartDiscIdRequest(const QString& discid) {
  typedef QPair<QString, QString> Param;

  QList<Param> parameters;
  parameters << Param("type", "xml")
             << Param("discid", discid);

  QUrl url(kDiscUrl);
  url.setQueryItems(parameters);
  QNetworkRequest req(url);

  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(DiscIdRequestFinished()));
  //requests_[reply] = id;

  timeouts_->AddReply(reply);
}

void MusicBrainzClient::Cancel(int id) {
  QNetworkReply* reply = requests_.key(id);
  requests_.remove(reply);
  delete reply;
}

void MusicBrainzClient::CancelAll() {
  qDeleteAll(requests_.keys());
  requests_.clear();
}

void MusicBrainzClient::DiscIdRequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;
  reply->deleteLater();

  ResultList ret;
  QString artist;
  QString album;

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
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
    if (token == QXmlStreamReader::StartElement && reader.name() == "track") {
      Result track = ParseTrack(&reader);
      if (!track.title_.isEmpty()) {
        ret << track;
      }
    } else if (token == QXmlStreamReader::EndElement && reader.name() == "track-list") {
      break;
    }
  }

  emit Finished(artist, album, ret);
}


void MusicBrainzClient::RequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();
  if (!requests_.contains(reply))
    return;

  int id = requests_.take(reply);
  ResultList ret;

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
    emit Finished(id, ret);
    return;
  }

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "track") {
      Result track = ParseTrack(&reader);
      if (!track.title_.isEmpty()) {
        ret << track;
      }
    }
  }

  emit Finished(id, ret);
}

MusicBrainzClient::Result MusicBrainzClient::ParseTrack(QXmlStreamReader* reader) {
  Result ret;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();

      if (name == "title") {
        ret.title_ = reader->readElementText();
      } else if (name == "duration") {
        ret.duration_msec_ = reader->readElementText().toInt();
      } else if (name == "artist") {
        ParseArtist(reader, &ret.artist_);
      } else if (name == "release") {
        ParseAlbum(reader, &ret.album_, &ret.track_);
      }
    }

    if (type == QXmlStreamReader::EndElement && reader->name() == "track") {
      break;
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

void MusicBrainzClient::ParseAlbum(QXmlStreamReader* reader, QString* album, int* track) {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      if (name == "title") {
        *album = reader->readElementText();
      } else if (name == "track-list") {
        *track = reader->attributes().value("offset").toString().toInt() + 1;
      }
    }

    if (type == QXmlStreamReader::EndElement && reader->name() == "release") {
      return;
    }
  }
}
