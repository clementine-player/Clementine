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

#include <algorithm>

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
const int MusicBrainzClient::kMaxRequestPerTrack = 3;

MusicBrainzClient::MusicBrainzClient(QObject* parent,
                                     QNetworkAccessManager* network)
    : QObject(parent),
      network_(network ? network : new NetworkAccessManager(this)),
      timeouts_(new NetworkTimeouts(kDefaultTimeout, this)) {}

void MusicBrainzClient::Start(int id, const QStringList& mbid_list) {
  typedef QPair<QString, QString> Param;

  int request_number = 0;
  for (const QString& mbid : mbid_list) {
    QList<Param> parameters;
    parameters << Param("inc", "artists+releases+media");

    QUrl url(kTrackUrl + mbid);
    url.setQueryItems(parameters);
    QNetworkRequest req(url);

    QNetworkReply* reply = network_->get(req);
    NewClosure(reply, SIGNAL(finished()), this,
               SLOT(RequestFinished(QNetworkReply*, int, int)), reply, id,
               request_number++);
    requests_.insert(id, reply);

    timeouts_->AddReply(reply);

    if (request_number >= kMaxRequestPerTrack) {
      break;
    }
  }
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
             SLOT(DiscIdRequestFinished(const QString&, QNetworkReply*)),
             discid, reply);

  timeouts_->AddReply(reply);
}

void MusicBrainzClient::Cancel(int id) { delete requests_.take(id); }

void MusicBrainzClient::CancelAll() {
  qDeleteAll(requests_.values());
  requests_.clear();
}

void MusicBrainzClient::DiscIdRequestFinished(const QString& discid,
                                              QNetworkReply* reply) {
  reply->deleteLater();

  ResultList ret;
  QString artist;
  QString album;
  int year = 0;

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() !=
      200) {
    qLog(Error) << "Error:"
                << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                       .toInt() << "http status code received";
    qLog(Error) << reply->readAll();
    emit Finished(artist, album, ret);
    return;
  }

  // Parse xml result:
  // -get title
  // -get artist
  // -get year
  // -get all the tracks' tags
  // Note: If there are multiple releases for the discid, the first
  // release is chosen.
  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    QXmlStreamReader::TokenType type = reader.readNext();
    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader.name();
      if (name == "title") {
        album = reader.readElementText();
      } else if (name == "date") {
        QRegExp regex(kDateRegex);
        if (regex.indexIn(reader.readElementText()) == 0) {
          year = regex.cap(0).toInt();
        }
      } else if (name == "artist-credit") {
        ParseArtist(&reader, &artist);
      } else if (name == "medium-list") {
        break;
      }
    }
  }

  while (!reader.atEnd()) {
    QXmlStreamReader::TokenType token = reader.readNext();
    if (token == QXmlStreamReader::StartElement && reader.name() == "medium") {
      // Get the medium with a matching discid.
      if (MediumHasDiscid(discid, &reader)) {
        ResultList tracks = ParseMedium(&reader);
        for (const Result& track : tracks) {
          if (!track.title_.isEmpty()) {
            ret << track;
          }
        }
      } else {
        Utilities::ConsumeCurrentElement(&reader);
      }
    } else if (token == QXmlStreamReader::EndElement &&
               reader.name() == "medium-list") {
      break;
    }
  }

  // If we parsed a year, copy it to the tracks.
  if (year > 0) {
    for (ResultList::iterator it = ret.begin(); it != ret.end(); ++it) {
      it->year_ = year;
    }
  }

  emit Finished(artist, album, UniqueResults(ret, SortResults));
}

void MusicBrainzClient::RequestFinished(QNetworkReply* reply, int id,
                                        int request_number) {
  reply->deleteLater();

  const int nb_removed = requests_.remove(id, reply);
  if (nb_removed != 1) {
    qLog(Error)
        << "Error: unknown reply received:" << nb_removed
        << "requests removed, while only one was supposed to be removed";
  }

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() ==
      200) {
    QXmlStreamReader reader(reply);
    ResultList res;
    while (!reader.atEnd()) {
      if (reader.readNext() == QXmlStreamReader::StartElement &&
          reader.name() == "recording") {
        ResultList tracks = ParseTrack(&reader);
        for (const Result& track : tracks) {
          if (!track.title_.isEmpty()) {
            res << track;
          }
        }
      }
    }
    pending_results_[id] << PendingResults(request_number, res);
  } else {
    qLog(Error) << "Error:"
                << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                       .toInt() << "http status code received";
    qLog(Error) << reply->readAll();
  }

  // No more pending requests for this id: emit the results we have.
  if (!requests_.contains(id)) {
    // Merge the results we have
    ResultList ret;
    QList<PendingResults> result_list_list = pending_results_.take(id);
    std::sort(result_list_list.begin(), result_list_list.end());
    for (const PendingResults& result_list : result_list_list) {
      ret << result_list.results_;
    }
    emit Finished(id, UniqueResults(ret, KeepOriginalOrder));
  }
}

bool MusicBrainzClient::MediumHasDiscid(const QString& discid,
                                        QXmlStreamReader* reader) {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement && reader->name() == "disc" &&
        reader->attributes().value("id").toString() == discid) {
      return true;
    } else if (type == QXmlStreamReader::EndElement &&
               reader->name() == "disc-list") {
      return false;
    }
  }
  qLog(Debug) << "Reached end of xml stream without encountering </disc-list>";
  return false;
}

MusicBrainzClient::ResultList MusicBrainzClient::ParseMedium(
    QXmlStreamReader* reader) {
  ResultList ret;
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement) {
      if (reader->name() == "track") {
        Result result;
        result = ParseTrackFromDisc(reader);
        ret << result;
      }
    }

    if (type == QXmlStreamReader::EndElement &&
        reader->name() == "track-list") {
      break;
    }
  }

  return ret;
}

MusicBrainzClient::Result MusicBrainzClient::ParseTrackFromDisc(
    QXmlStreamReader* reader) {
  Result result;

  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      if (name == "position") {
        result.track_ = reader->readElementText().toInt();
      } else if (name == "length") {
        result.duration_msec_ = reader->readElementText().toInt();
      } else if (name == "title") {
        result.title_ = reader->readElementText();
      }
    }

    if (type == QXmlStreamReader::EndElement && reader->name() == "track") {
      break;
    }
  }

  return result;
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
      } else if (name == "artist-credit") {
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
    std::stable_sort(releases.begin(), releases.end());
    for (const Release& release : releases) {
      ret << release.CopyAndMergeInto(result);
    }
  }
  return ret;
}

// Parse the artist. Multiple artists are joined together with the
// joinphrase from musicbrainz.
void MusicBrainzClient::ParseArtist(QXmlStreamReader* reader, QString* artist) {
  QString join_phrase;
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();

    if (type == QXmlStreamReader::StartElement &&
        reader->name() == "name-credit") {
      join_phrase = reader->attributes().value("joinphrase").toString();
    }

    if (type == QXmlStreamReader::StartElement && reader->name() == "name") {
      *artist += reader->readElementText() + join_phrase;
    }

    if (type == QXmlStreamReader::EndElement &&
        reader->name() == "artist-credit") {
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
      } else if (name == "status") {
        ret.SetStatusFromString(reader->readElementText());
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
    const ResultList& results, UniqueResultsSortOption opt) {
  ResultList ret;
  if (opt == SortResults) {
    ret = QSet<Result>::fromList(results).toList();
    std::sort(ret.begin(), ret.end());
  } else {  // KeepOriginalOrder
    // Qt doesn't provide a ordered set (QSet "stores values in an unspecified
    // order" according to Qt documentation).
    // We might use std::set instead, but it's probably faster to use ResultList
    // directly to avoid converting from one structure to another.
    for (const Result& res : results) {
      if (!ret.contains(res)) {
        ret << res;
      }
    }
  }
  return ret;
}
