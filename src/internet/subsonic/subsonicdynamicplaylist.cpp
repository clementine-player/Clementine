/* This file is part of Clementine.
   Copyright 2015, Nick Lanham <nick@afternight.org>

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

#include "subsonicdynamicplaylist.h"
#include "subsonicservice.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QSslConfiguration>
#include <QUrlQuery>

#include "core/application.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/waitforsignal.h"
#include "internet/core/internetplaylistitem.h"

#include <boost/scope_exit.hpp>

// 500 limit per subsonic api
const int SubsonicDynamicPlaylist::kMaxCount = 500;
const int SubsonicDynamicPlaylist::kDefaultAlbumCount = 10;
const int SubsonicDynamicPlaylist::kDefaultSongCount = 20;
const int SubsonicDynamicPlaylist::kDefaultOffset = 0;

SubsonicDynamicPlaylist::SubsonicDynamicPlaylist()
  : type_(QueryType_Album), stat_(QueryStat_Newest), offset_(kDefaultOffset) {
  service_ = InternetModel::Service<SubsonicService>();
}

SubsonicDynamicPlaylist::SubsonicDynamicPlaylist(const QString& name,
                                                 QueryType type,
                                                 QueryStat stat)
    : type_(type), stat_(stat), offset_(kDefaultOffset) {
  set_name(name);
  service_ = InternetModel::Service<SubsonicService>();
}

void SubsonicDynamicPlaylist::Load(const QByteArray& data) {
  QDataStream s(data);
  s >> *this;
}

void SubsonicDynamicPlaylist::Load(QueryStat stat) { stat_ = stat; }

QByteArray SubsonicDynamicPlaylist::Save() const {
  QByteArray ret;
  QDataStream s(&ret, QIODevice::WriteOnly);
  s << *this;
  return ret;
}

// copied from SubsonicService
QNetworkReply* SubsonicDynamicPlaylist::Send(QNetworkAccessManager& network,
                                             const QUrl& url,
                                             const bool usesslv3) {
  QNetworkRequest request(url);
  // Don't try and check the authenticity of the SSL certificate - it'll almost
  // certainly be self-signed.
  QSslConfiguration sslconfig = QSslConfiguration::defaultConfiguration();
  sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  if (usesslv3) {
    sslconfig.setProtocol(QSsl::SslV3);
  }
  request.setSslConfiguration(sslconfig);
  QNetworkReply* reply = network.get(request);
  return reply;
}

PlaylistItemList SubsonicDynamicPlaylist::Generate() {
  switch (type_) {
    case QueryType_Album:
      return GenerateMoreAlbums(kDefaultAlbumCount);
    case QueryType_Song:
      return GenerateMoreSongs(kDefaultSongCount);
    default:
      qLog(Warning) << "Invalid playlist type";
      return PlaylistItemList();
  }
}

PlaylistItemList SubsonicDynamicPlaylist::GenerateMoreSongs(int count) {
  const int task_id =
      service_->app_->task_manager()->StartTask(tr("Fetching playlist items"));
  TaskManager::ScopedTask task(task_id, service_->app_->task_manager());

  QUrl url = service_->BuildRequestUrl("getRandomSongs");
  QNetworkAccessManager network;

  if (count > kMaxCount) count = kMaxCount;

  QUrlQuery url_query(url.query());
  url_query.addQueryItem("size", QString::number(count));
  url.setQuery(url_query);

  PlaylistItemList items;

  QNetworkReply* reply = Send(network, url, service_->usesslv3_);
  WaitForSignal(reply, SIGNAL(finished()));

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "HTTP error returned from Subsonic:"
                  << reply->errorString() << ", url:" << url.toString();
    return items;  // empty
  }

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();
  if (reader.name() != "subsonic-response") {
    qLog(Warning) << "Not a subsonic-response, aboring playlist fetch";
    return items;
  }

  if (reader.attributes().value("status") != "ok") {
    reader.readNextStartElement();
    int error = reader.attributes().value("code").toString().toInt();
    qLog(Warning) << "An error occurred fetching data.  Code: " << error
                  << " Message: "
                  << reader.attributes().value("message").toString();
    return items;
  }

  reader.readNextStartElement();
  if (reader.name() != "randomSongs") {
    qLog(Warning) << "randomSongs tag expected.  Aborting playlist fetch";
    return items;
  }

  while (reader.readNextStartElement()) {
    if (reader.name() != "song") {
      qLog(Warning) << "song tag expected. Skipping song";
      reader.skipCurrentElement();
      continue;
    }

    Song song = service_->ReadSong(reader);

    items << std::shared_ptr<PlaylistItem>(
        new InternetPlaylistItem(service_, song));

    reader.skipCurrentElement();
  }
  return items;
}

PlaylistItemList SubsonicDynamicPlaylist::GenerateMoreAlbums(int count) {
  const int task_id =
      service_->app_->task_manager()->StartTask(tr("Fetching playlist items"));
  TaskManager::ScopedTask task(task_id, service_->app_->task_manager());

  QUrl url = service_->BuildRequestUrl("getAlbumList");
  QNetworkAccessManager network;

  if (count > kMaxCount) count = kMaxCount;

  QUrlQuery url_query(url.query());
  url_query.addQueryItem("type", GetTypeString());
  url_query.addQueryItem("size", QString::number(count));
  url_query.addQueryItem("offset", QString::number(offset_));
  url.setQuery(url_query);

  PlaylistItemList items;

  QNetworkReply* reply = Send(network, url, service_->usesslv3_);
  WaitForSignal(reply, SIGNAL(finished()));

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "HTTP error returned from Subsonic:"
                  << reply->errorString() << ", url:" << url.toString();
    return items;  // empty
  }

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();
  if (reader.name() != "subsonic-response") {
    qLog(Warning) << "Not a subsonic-response, aboring playlist fetch";
    return items;
  }

  if (reader.attributes().value("status") != "ok") {
    reader.readNextStartElement();
    int error = reader.attributes().value("code").toString().toInt();
    qLog(Warning) << "An error occurred fetching data.  Code: " << error
                  << " Message: "
                  << reader.attributes().value("message").toString();
    return items;
  }

  reader.readNextStartElement();
  if (reader.name() != "albumList") {
    qLog(Warning) << "albumList tag expected.  Aboring playlist fetch";
    return items;
  }

  while (reader.readNextStartElement()) {
    if (reader.name() != "album") {
      qLog(Warning) << "album tag expected. Skipping album";
      reader.skipCurrentElement();
      continue;
    }

    qLog(Debug) << "Getting album: "
                << reader.attributes().value("album").toString();
    GetAlbum(items, reader.attributes().value("id").toString(), network,
             service_->usesslv3_);
    reader.skipCurrentElement();
  }
  offset_ += count;
  return items;
}

void SubsonicDynamicPlaylist::GetAlbum(PlaylistItemList& list, QString id,
                                       QNetworkAccessManager& network,
                                       const bool usesslv3) {
  QUrl url = service_->BuildRequestUrl("getAlbum");
  QUrlQuery url_query(url.query());
  url_query.addQueryItem("id", id);
  if (service_->IsAmpache()) {
    url_query.addQueryItem("ampache", "1");
  }
  url.setQuery(url_query);
  QNetworkReply* reply = Send(network, url, usesslv3);
  WaitForSignal(reply, SIGNAL(finished()));
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "HTTP error returned from Subsonic:"
                  << reply->errorString() << ", url:" << url.toString();
    return;
  }

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();

  if (reader.name() != "subsonic-response") {
    qLog(Warning) << "Not a subsonic-response. Aborting playlist fetch.";
    return;
  }

  if (reader.attributes().value("status") != "ok") {
    qLog(Warning) << "Status not okay. Aborting playlist fetch.";
    return;
  }

  // Read album information
  reader.readNextStartElement();
  if (reader.name() != "album") {
    qLog(Warning) << "album tag expected. Aborting playlist fetch.";
    return;
  }

  QString album_artist = reader.attributes().value("artist").toString();

  // Read song information
  while (reader.readNextStartElement()) {
    if (reader.name() != "song") {
      qLog(Warning) << "song tag expected. Skipping song";
      reader.skipCurrentElement();
      continue;
    }

    Song song = service_->ReadSong(reader);
    song.set_albumartist(album_artist);

    list << std::shared_ptr<PlaylistItem>(
        new InternetPlaylistItem(service_, song));

    reader.skipCurrentElement();
  }
}

QDataStream& operator<<(QDataStream& s, const SubsonicDynamicPlaylist& p) {
  s << quint8(p.stat_) << quint8(p.type_);
  return s;
}

QDataStream& operator>>(QDataStream& s, SubsonicDynamicPlaylist& p) {
  quint8 stat, type;
  s >> stat >> type;
  p.stat_ = SubsonicDynamicPlaylist::QueryStat(stat);
  p.type_ = SubsonicDynamicPlaylist::QueryType(type);
  return s;
}
