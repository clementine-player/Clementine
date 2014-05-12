/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "jamendodynamicplaylist.h"

#include "jamendoplaylistitem.h"
#include "jamendoservice.h"

#include <QEventLoop>
#include <QHttp>
#include <QHttpRequestHeader>
#include <QtDebug>

#include "core/logging.h"
#include "core/network.h"
#include "library/librarybackend.h"

const char* JamendoDynamicPlaylist::kUrl =
    "http://api.jamendo.com/get2/id/track/plain/";

JamendoDynamicPlaylist::JamendoDynamicPlaylist()
    : order_by_(OrderBy_Rating),
      order_direction_(Order_Descending),
      current_page_(0),
      current_index_(0) {}

JamendoDynamicPlaylist::JamendoDynamicPlaylist(const QString& name,
                                               OrderBy order_by)
    : order_by_(order_by),
      order_direction_(Order_Descending),
      current_page_(0),
      current_index_(0) {
  set_name(name);
}

void JamendoDynamicPlaylist::Load(const QByteArray& data) {
  QDataStream s(data);
  s >> *this;
}

void JamendoDynamicPlaylist::Load(OrderBy order_by,
                                  OrderDirection order_direction) {
  order_by_ = order_by;
  order_direction_ = order_direction;
}

QByteArray JamendoDynamicPlaylist::Save() const {
  QByteArray ret;
  QDataStream s(&ret, QIODevice::WriteOnly);
  s << *this;

  return ret;
}

PlaylistItemList JamendoDynamicPlaylist::Generate() { return GenerateMore(20); }

PlaylistItemList JamendoDynamicPlaylist::GenerateMore(int count) {
  int tries = 0;

  PlaylistItemList items;
  while (items.size() < count && tries++ < kApiRetryLimit) {
    // Add items from current list.
    if (current_index_ < current_items_.size()) {
      PlaylistItemList more_items = current_items_.mid(current_index_, count);
      items << more_items;
      current_index_ += more_items.size();
    } else {
      // We need more songs!
      Fetch();
    }
  }

  return items;
}

QString JamendoDynamicPlaylist::OrderSpec(OrderBy by, OrderDirection dir) {
  QString ret;
  switch (by) {
    case OrderBy_Listened:
      ret += "listened";
      break;
    case OrderBy_Rating:
      ret += "rating";
      break;
    case OrderBy_RatingMonth:
      ret += "ratingmonth";
      break;
    case OrderBy_RatingWeek:
      ret += "ratingweek";
      break;
  }
  switch (dir) {
    case Order_Ascending:
      ret += "_asc";
      break;
    case Order_Descending:
      ret += "_desc";
      break;
  }
  return ret;
}

void JamendoDynamicPlaylist::Fetch() {
  QUrl url(kUrl);
  url.addQueryItem("pn", QString::number(current_page_++));
  url.addQueryItem("n", QString::number(kPageSize));
  url.addQueryItem("order", OrderSpec(order_by_, order_direction_));

  // We have to use QHttp here because there's no way to disable Keep-Alive
  // with QNetworkManager.
  QHttpRequestHeader header(
      "GET", QString(url.encodedPath() + "?" + url.encodedQuery()));
  header.setValue("Host", url.encodedHost());

  QHttp http(url.host());
  http.request(header);

  // Wait for the reply
  {
    QEventLoop event_loop;
    connect(&http, SIGNAL(requestFinished(int, bool)), &event_loop,
            SLOT(quit()));
    event_loop.exec();
  }

  if (http.error() != QHttp::NoError) {
    qLog(Warning) << "HTTP error returned from Jamendo:" << http.errorString()
                  << ", url:" << url.toString();
    return;
  }

  // The reply will contain one track ID per line
  QStringList lines = QString::fromAscii(http.readAll()).split('\n');

  // Get the songs from the database
  SongList songs = backend_->GetSongsByForeignId(
      lines, JamendoService::kTrackIdsTable, JamendoService::kTrackIdsColumn);

  if (songs.empty()) {
    qLog(Warning) << "No songs returned from Jamendo:" << url.toString();
    return;
  }

  current_items_.clear();
  for (const Song& song : songs) {
    if (song.is_valid())
      current_items_ << PlaylistItemPtr(new JamendoPlaylistItem(song));
  }
  current_index_ = 0;
}

QDataStream& operator<<(QDataStream& s, const JamendoDynamicPlaylist& p) {
  s << quint8(p.order_by_) << quint8(p.order_direction_);
  return s;
}

QDataStream& operator>>(QDataStream& s, JamendoDynamicPlaylist& p) {
  quint8 order_by, order_direction;
  s >> order_by >> order_direction;
  p.order_by_ = JamendoDynamicPlaylist::OrderBy(order_by);
  p.order_direction_ = JamendoDynamicPlaylist::OrderDirection(order_direction);
  return s;
}
