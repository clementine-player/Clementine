/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "plexlibraryscanner.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrlQuery>

#include "core/closure.h"
#include "core/logging.h"
#include "internet/plex/plexparser.h"
#include "internet/plex/plexservice.h"

const int PlexLibraryScanner::kPageSize = 500;

PlexLibraryScanner::PlexLibraryScanner(PlexService* service, QObject* parent)
    : QObject(parent),
      service_(service),
      scanning_(false),
      done_(0),
      total_(0) {}

void PlexLibraryScanner::Scan(const QStringList& section_keys) {
  if (scanning_) return;

  scanning_ = true;
  pending_sections_ = section_keys;
  songs_.clear();
  done_ = 0;
  total_ = 0;
  NextSection();
}

void PlexLibraryScanner::NextSection() {
  if (pending_sections_.isEmpty()) {
    Finish(true);
    return;
  }

  current_section_ = pending_sections_.takeFirst();
  songs_[current_section_] = SongList();
  RequestPage(0);
}

void PlexLibraryScanner::RequestPage(int offset) {
  QUrlQuery query;
  query.addQueryItem("type", "10");
  // A unique sort key keeps pages stable while we walk them.  Plex rejects
  // sort=ratingKey with a 500 but accepts id, which is the same number.
  query.addQueryItem("sort", "id");
  query.addQueryItem("X-Plex-Container-Start", QString::number(offset));
  query.addQueryItem("X-Plex-Container-Size", QString::number(kPageSize));

  QNetworkReply* reply = service_->Get(
      QString("/library/sections/%1/all").arg(current_section_), query);
  NewClosure(reply, &QNetworkReply::finished, this,
             &PlexLibraryScanner::PageFinished, reply, offset);
}

void PlexLibraryScanner::PageFinished(QNetworkReply* reply, int offset) {
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "Plex section" << current_section_
                  << "fetch failed:" << reply->errorString();
    Finish(false);
    return;
  }

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    qLog(Warning) << "Plex section" << current_section_
                  << "returned bad JSON:" << error.errorString();
    Finish(false);
    return;
  }

  PlexTracksPage page =
      PlexParser::ParseTracks(doc, service_->machine_id(), current_section_,
                              service_->base_url(), service_->server_token());
  songs_[current_section_] << page.songs;

  if (offset == 0) total_ += page.total_size;
  done_ += page.size;
  emit Progress(done_, total_);

  const int next = offset + page.size;
  if (page.size > 0 && next < page.total_size) {
    RequestPage(next);
  } else {
    NextSection();
  }
}

void PlexLibraryScanner::Finish(bool ok) {
  scanning_ = false;
  pending_sections_.clear();
  QMap<QString, SongList> songs;
  songs.swap(songs_);
  emit ScanFinished(songs, ok);
}
