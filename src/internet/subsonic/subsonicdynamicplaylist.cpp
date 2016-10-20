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
#include <QXmlStreamReader>

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
const int SubsonicDynamicPlaylist::kDefaultCount = 10;
const int SubsonicDynamicPlaylist::kDefaultOffset = 0;

SubsonicDynamicPlaylist::SubsonicDynamicPlaylist()
  : stat_(QueryStat_Newest), offset_(kDefaultOffset) {}

SubsonicDynamicPlaylist::SubsonicDynamicPlaylist(const QString& name,
                                                 QueryStat stat)
    : stat_(stat), offset_(kDefaultOffset) {
  set_name(name);
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
  return GenerateMore(kDefaultCount);
}

PlaylistItemList SubsonicDynamicPlaylist::GenerateMore(int count) {
  SubsonicService* service = InternetModel::Service<SubsonicService>();
  const int task_id =
      service->app_->task_manager()->StartTask(tr("Fetching Playlist Items"));

  BOOST_SCOPE_EXIT((service)(task_id)) {
    // stop task when we're done
    service->app_->task_manager()->SetTaskFinished(task_id);
  }
  BOOST_SCOPE_EXIT_END

  QUrl url = service->BuildRequestUrl("getAlbumList");
  QNetworkAccessManager network;

  if (count > kMaxCount) count = kMaxCount;

  url.addQueryItem("type", GetTypeString());
  url.addQueryItem("size", QString::number(count));
  url.addQueryItem("offset", QString::number(offset_));

  PlaylistItemList items;

  QNetworkReply* reply = Send(network, url, service->usesslv3_);
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
    qLog(Warning) << "An error occured fetching data.  Code: " << error
                  << " Message: "
                  << reader.attributes().value("message").toString();
  }

  reader.readNextStartElement();
  if (reader.name() != "albumList") {
    qLog(Warning) << "albumList tag expected.  Aboring playlist fetch";
    return items;
  }

  while (reader.readNextStartElement()) {
    if (reader.name() != "album") {
      qLog(Warning) << "album tag expected. Aboring playlist fetch";
      return items;
    }

    qLog(Debug) << "Getting album: "
                << reader.attributes().value("album").toString();
    GetAlbum(service, items, reader.attributes().value("id").toString(),
             network, service->usesslv3_);
    reader.skipCurrentElement();
  }
  offset_ += count;
  return items;
}

void SubsonicDynamicPlaylist::GetAlbum(SubsonicService* service,
                                       PlaylistItemList& list, QString id,
                                       QNetworkAccessManager& network,
                                       const bool usesslv3) {
  QUrl url = service->BuildRequestUrl("getAlbum");
  url.addQueryItem("id", id);
  if (service->IsAmpache()) {
    url.addQueryItem("ampache", "1");
  }
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
      qLog(Warning) << "song tag expected. Aborting playlist fetch.";
      return;
    }

    Song song;
    QString id = reader.attributes().value("id").toString();
    song.set_title(reader.attributes().value("title").toString());
    song.set_album(reader.attributes().value("album").toString());
    song.set_track(reader.attributes().value("track").toString().toInt());
    song.set_disc(reader.attributes().value("discNumber").toString().toInt());
    song.set_artist(reader.attributes().value("artist").toString());
    song.set_albumartist(album_artist);
    song.set_bitrate(reader.attributes().value("bitRate").toString().toInt());
    song.set_year(reader.attributes().value("year").toString().toInt());
    song.set_genre(reader.attributes().value("genre").toString());
    qint64 length = reader.attributes().value("duration").toString().toInt();
    length *= kNsecPerSec;
    song.set_length_nanosec(length);
    QUrl url = QUrl(QString("subsonic://%1").arg(id));
    song.set_url(url);
    song.set_filesize(reader.attributes().value("size").toString().toInt());
    QFileInfo fi(reader.attributes().value("path").toString());
    song.set_basefilename(fi.fileName());
    // We need to set these to satisfy the database constraints
    song.set_directory_id(0);
    song.set_mtime(0);
    song.set_ctime(0);

    if (reader.attributes().hasAttribute("playCount")) {
      song.set_playcount(
          reader.attributes().value("playCount").toString().toInt());
    }

    list << std::shared_ptr<PlaylistItem>(
        new InternetPlaylistItem(service, song));

    reader.skipCurrentElement();
  }
}

QDataStream& operator<<(QDataStream& s, const SubsonicDynamicPlaylist& p) {
  s << quint8(p.stat_);
  return s;
}

QDataStream& operator>>(QDataStream& s, SubsonicDynamicPlaylist& p) {
  quint8 stat;
  s >> stat;
  p.stat_ = SubsonicDynamicPlaylist::QueryStat(stat);
  return s;
}
