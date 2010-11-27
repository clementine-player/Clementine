#include "jamendodynamicplaylist.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QtDebug>

#include "core/network.h"
#include "library/librarybackend.h"
#include "radio/jamendoplaylistitem.h"
#include "radio/jamendoservice.h"

const char* JamendoDynamicPlaylist::kUrl =
    "http://api.jamendo.com/get2/id/track/plain/";

JamendoDynamicPlaylist::JamendoDynamicPlaylist()
  : order_by_(OrderBy_Rating),
    order_direction_(Order_Descending),
    current_page_(0),
    current_index_(0) {
}

JamendoDynamicPlaylist::JamendoDynamicPlaylist(const QString& name, OrderBy order_by)
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

void JamendoDynamicPlaylist::Load(OrderBy order_by, OrderDirection order_direction) {
  order_by_ = order_by;
  order_direction_ = order_direction;
}

QByteArray JamendoDynamicPlaylist::Save() const {
  QByteArray ret;
  QDataStream s(&ret, QIODevice::WriteOnly);
  s << *this;

  return ret;
}

PlaylistItemList JamendoDynamicPlaylist::Generate() {
  return GenerateMore(20);
}

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
    case OrderBy_Listened:    ret += "listened";    break;
    case OrderBy_Rating:      ret += "rating";      break;
    case OrderBy_RatingMonth: ret += "ratingmonth"; break;
    case OrderBy_RatingWeek:  ret += "ratingweek";  break;
  }
  switch (dir) {
    case Order_Ascending:     ret += "_asc";        break;
    case Order_Descending:    ret += "_desc";       break;
  }
  return ret;
}

void JamendoDynamicPlaylist::Fetch() {
  QUrl url(kUrl);
  url.addQueryItem("pn", QString::number(current_page_++));
  url.addQueryItem("n", QString::number(kPageSize));
  url.addQueryItem("order", OrderSpec(order_by_, order_direction_));

  // Have to make a new NetworkAccessManager here because we're in a different
  // thread.
  NetworkAccessManager network;
  QNetworkRequest req(url);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network.get(req);

  // Blocking wait for reply.
  {
    QEventLoop event_loop;
    connect(reply, SIGNAL(finished()), &event_loop, SLOT(quit()));
    event_loop.exec();
  }

  // The reply will contain one track ID per line
  QStringList lines = QString::fromAscii(reply->readAll()).split('\n');
  delete reply;

  // Get the songs from the database
  SongList songs = backend_->GetSongsByForeignId(
        lines, JamendoService::kTrackIdsTable, JamendoService::kTrackIdsColumn);

  if (songs.empty()) {
    qWarning() << "No songs returned from Jamendo:"
               << url.toString();
    return;
  }

  current_items_.clear();
  foreach (const Song& song, songs) {
    if (song.is_valid())
      current_items_ << PlaylistItemPtr(new JamendoPlaylistItem(song));
  }
  current_index_ = 0;
}

QDataStream& operator <<(QDataStream& s, const JamendoDynamicPlaylist& p) {
  s << quint8(p.order_by_) << quint8(p.order_direction_);
  return s;
}

QDataStream& operator >>(QDataStream& s, JamendoDynamicPlaylist& p) {
  quint8 order_by, order_direction;
  s >> order_by >> order_direction;
  p.order_by_ = JamendoDynamicPlaylist::OrderBy(order_by);
  p.order_direction_ = JamendoDynamicPlaylist::OrderDirection(order_direction);
  return s;
}
