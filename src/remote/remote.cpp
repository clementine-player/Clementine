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

#include "remote.h"
#include "remotesettingspage.h"
#include "core/player.h"
#include "engines/enginebase.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"

#include <QDataStream>
#include <QHostAddress>
#include <QSettings>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>

Remote::Remote(Player* player, QObject* parent)
  : QObject(parent),
    player_(player),
    connection_(new xrme::Connection(this)),
    retry_count_(0)
{
  connection_->SetMediaPlayer(this);
  connection_->SetMediaStorage(this);
  connection_->set_verbose(true);
  connect(connection_, SIGNAL(Connected()), SLOT(Connected()));
  connect(connection_, SIGNAL(Disconnected(QString)), SLOT(Disconnected(QString)));

  connect(player_, SIGNAL(Playing()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(Paused()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(Stopped()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(PlaylistFinished()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(VolumeChanged(int)), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(Seeked(qlonglong)), SLOT(SetStateChanged()));
  connect(player_->playlists(), SIGNAL(CurrentSongChanged(Song)), SLOT(SetStateChanged()));

  connect(connection_,
      SIGNAL(TomahawkSIPReceived(const QVariant&)),
      SLOT(TomahawkSIPReceived(const QVariant&)));

  ReloadSettings();
}

void Remote::ReloadSettings() {
  QSettings s;
  s.beginGroup(RemoteSettingsPage::kSettingsGroup);

  QString username = s.value("username").toString();
  QString password = s.value("password").toString();
  QString agent_name = s.value("agent_name", RemoteSettingsPage::DefaultAgentName()).toString();

  // Have the settings changed?
  if (username != connection_->username() ||
      password != connection_->password() ||
      agent_name != connection_->agent_name()) {
    connection_->set_username(username);
    connection_->set_agent_name(agent_name);
    connection_->set_password(password);

    if (connection_->is_connected()) {
      // We'll reconnect later
      connection_->Disconnect();
    } else if (is_configured()) {
      connection_->Connect();
    }
  }
}

bool Remote::is_configured() const {
  return !connection_->username().isEmpty() &&
         !connection_->password().isEmpty() &&
         !connection_->agent_name().isEmpty();
}

void Remote::Connected() {
  retry_count_ = 0;
}

void Remote::Disconnected(const QString& error) {
  if (retry_count_++ >= kMaxRetries) {
    // Show an error and give up if we're above the retry count
    if (!error.isEmpty()) {
      emit Error("XMPP remote control disconnected: " + error);
    }
  } else if (is_configured()) {
    // Try again
    QTimer::singleShot(0, connection_, SLOT(Connect()));
  }
}

void Remote::PlayPause() {
  player_->PlayPause();
}

void Remote::Stop() {
  player_->Stop();
}

void Remote::Next() {
  player_->Next();
}

void Remote::Previous() {
  player_->Previous();
}

xrme::State Remote::state() const {
  const Playlist* active = player_->playlists()->active();
  const PlaylistItemPtr current_item = player_->GetCurrentItem();

  xrme::State ret;
  ret.can_go_next = active->next_row() != -1;
  ret.can_go_previous = active->previous_row() != -1;
  ret.can_seek = current_item && !current_item->Metadata().is_stream();

  switch (player_->GetState()) {
    case Engine::Playing: ret.playback_state = xrme::State::PlaybackState_Playing; break;
    case Engine::Paused:  ret.playback_state = xrme::State::PlaybackState_Paused;  break;
    case Engine::Idle:
    case Engine::Empty:   ret.playback_state = xrme::State::PlaybackState_Stopped; break;
  }

  ret.position_millisec = player_->engine()->position_nanosec() / kNsecPerMsec;
  ret.volume = double(player_->GetVolume()) / 100;

  if (current_item) {
    const Song m = current_item->Metadata();

    ret.metadata.title = m.title();
    ret.metadata.artist = m.artist();
    ret.metadata.album = m.album();
    ret.metadata.albumartist = m.albumartist();
    ret.metadata.composer = m.composer();
    ret.metadata.genre = m.genre();
    ret.metadata.track = m.track();
    ret.metadata.disc = m.disc();
    ret.metadata.year = m.year();
    ret.metadata.length_millisec = m.length_nanosec() / kNsecPerMsec;
    ret.metadata.rating = m.rating();
  }

  return ret;
}

QImage Remote::album_art() const {
  return last_image_;
}

QStringList Remote::GetArtists() const {
  return QStringList();
}

void Remote::SetStateChanged() {
  StateChanged();
}

void Remote::ArtLoaded(const Song&, const QString&, const QImage& image) {
  last_image_ = image;
  AlbumArtChanged();
}

void Remote::TomahawkSIPReceived(const QVariant& json) {
  QVariantMap message = json.toMap();
  const QString& ip = message["ip"].toString();
  const QString& key = message["key"].toString();
  const quint16 port = message["port"].toUInt();
  const QString& unique_name = message["uniqname"].toString();
  const bool visible = message["visible"].toBool();

  QTcpSocket* socket = new QTcpSocket(this);
  socket->connectToHost(ip, port);
  connect(socket, SIGNAL(connected()), SLOT(TomahawkConnected()));
  connect(socket, SIGNAL(disconnected()), SLOT(TomahawkDisconnected()));
  connect(socket, SIGNAL(readyRead()), SLOT(TomahawkReadyRead()));
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                  SLOT(TomahawkError(QAbstractSocket::SocketError)));

  qDebug() << Q_FUNC_INFO << "connecting to" << ip << port;

  TomahawkConnection* connection = new TomahawkConnection;
  connection->key = key;
  connection->unique_name = unique_name;
  connection->visible = visible;
  connection->num_bytes = -1;
  tomahawk_connections_[socket] = connection;
}

void Remote::TomahawkConnected() {
  qDebug() << Q_FUNC_INFO;
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  Q_ASSERT(socket);

  TomahawkConnection* connection = tomahawk_connections_[socket];

  QVariantMap map;
  map["conntype"] = "accept-offer";
  map["key"] = connection->key;
  map["port"] = 12345;
  map["nodeid"] = "foobar";
  map["controlid"] = "foobarbaz";

  QJson::Serializer serialiser;
  const QByteArray& serialised = serialiser.serialize(map);
  quint32 length = serialised.length();

  // Send offer acceptance.
  {
    QDataStream stream(socket);
    stream << (quint32)length;
    stream << (quint8)(128 | 2);
    stream.writeRawData(serialised.constData(), serialised.length());
    qDebug() << "now:" << socket->bytesToWrite();
  }

  {
    QDataStream stream(socket);
    stream << (quint32)2;
    stream << (quint8)(128 | 1);
    stream.writeRawData("ok", 2);
  }

  // Send ping.
  {
    QDataStream stream(socket);
    stream << (quint32)0;
    stream << (quint8)(32);
  }

  /*
  {
    QVariantMap json;
    json["method"] = "dbsync-offer";
    json["key"] = "abcdefg";
    const QByteArray& request = serialiser.serialize(json);
    QDataStream stream(socket);
    stream << (quint32)request.length();
    stream << (quint8)2;  // JSON
    stream.writeRawData(request.constData(), request.length());
  }
  */
}

void Remote::TomahawkReadyRead() {
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  Q_ASSERT(socket);

  TomahawkConnection* connection = tomahawk_connections_[socket];
  if (connection->num_bytes == -1) {
    // Expecting header.
    if (socket->bytesAvailable() >= 5) {
      QDataStream stream(socket);
      quint32 size;
      stream >> size;
      quint8 flags;
      stream >> flags;
      connection->num_bytes = size > 0 ? size : -1;
      connection->flags = flags;
      if (flags & 32) {
        qDebug() << "PING!";
      }
      if (connection->num_bytes == -1) {
        return;
      }
    } else {
      return;
    }
  }

  if (socket->bytesAvailable() < connection->num_bytes) {
    return;
  }

  QByteArray data = socket->readAll();
  connection->num_bytes = -1;

  qDebug() << Q_FUNC_INFO;
  qDebug() << data;
  qDebug() << Q_FUNC_INFO << "done";

  if (connection->flags & 2) {
    QJson::Parser parser;
    bool ok = false;
    QVariant json = parser.parse(data, &ok);
    if (ok) {
      QVariantMap map = json.toMap();
      QString method = map["method"].toString();
      QString conntype = map["conntype"].toString();
      if (method == "dbsync-offer") {
        qDebug() << "DBSYNC!";
        TomahawkConnection* db_connection = new TomahawkConnection;
        db_connection->key = map["key"].toString();
        db_connection->num_bytes = -1;
        QTcpSocket* sync_socket = new QTcpSocket(this);
        sync_socket->connectToHost(socket->peerAddress(), socket->peerPort());
        connect(sync_socket, SIGNAL(connected()), SLOT(TomahawkDBConnected()));
        connect(sync_socket, SIGNAL(disconnected()), SLOT(TomahawkDBDisconnected()));
        connect(sync_socket, SIGNAL(readyRead()), SLOT(TomahawkDBReadyRead()));
        tomahawk_connections_[sync_socket] = db_connection;
      } else if (conntype == "request-offer") {
        qDebug() << "File Transfer!";
        TomahawkConnection* file_transfer = new TomahawkConnection;
        file_transfer->key = map["key"].toString();
        file_transfer->num_bytes = -1;
        file_transfer->offer = map["offer"].toString();
        file_transfer->controlid = map["controlid"].toString();
        QTcpSocket* file_transfer_socket = new QTcpSocket(this);
        file_transfer_socket->connectToHost(socket->peerAddress(), map["port"].toUInt());
        connect(file_transfer_socket, SIGNAL(connected()), SLOT(TomahawkTransferConnected()));
        connect(file_transfer_socket, SIGNAL(disconnected()), SLOT(TomahawkTransferDisconnected()));
        connect(file_transfer_socket, SIGNAL(readyRead()), SLOT(TomahawkTransferReadyRead()));
        tomahawk_connections_[file_transfer_socket] = file_transfer;
      }
    }
  }
}

void Remote::TomahawkDBConnected() {
  qDebug() << Q_FUNC_INFO;
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  Q_ASSERT(socket);

  TomahawkConnection* connection = tomahawk_connections_[socket];

  {
    QVariantMap request;
    request["method"] = "accept-offer";
    request["key"] = connection->key;

    QJson::Serializer serialiser;
    const QByteArray& json = serialiser.serialize(request);

    QDataStream stream(socket);
    stream << (quint32)json.length();
    stream << (quint8)(2);
    stream.writeRawData(json.constData(), json.length());
  }

  {
    QDataStream stream(socket);
    stream << (quint32)2;
    stream << (quint8)(128 | 1);  // SETUP | RAW
    stream.writeRawData("ok", 2);
  }

  {
    QVariantMap request;
    request["command"] = "addfiles";
    QVariantMap file;
    file["url"] = "http://data.clementine-player.org/rainymood";
    file["mtime"] = 123456;
    file["size"] = 1e7;
    file["hash"] = "abcdefg";
    file["mimetype"] = "audio/mpeg";
    file["duration"] = 1000;
    file["bitrate"] = 128;
    file["artist"] = "foo";
    file["album"] = "bar";
    file["track"] = "Rain!";
    file["albumpos"] = 1;
    file["year"] = 2011;
    QVariantList files;
    files << file;
    request["files"] = files;

    QJson::Serializer serialiser;
    const QByteArray& json = serialiser.serialize(request);

    qDebug() << json;

    QDataStream stream(socket);
    stream << (quint32)json.length();
    stream << (quint8)(16 | 2);  // DBOP | JSON
    stream.writeRawData(json.constData(), json.length());
  }
}

void Remote::TomahawkDBReadyRead() {
  qDebug() << Q_FUNC_INFO;
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  Q_ASSERT(socket);

  TomahawkConnection* connection = tomahawk_connections_[socket];
  if (connection->num_bytes == -1) {
    // Expecting header.
    if (socket->bytesAvailable() >= 5) {
      QDataStream stream(socket);
      quint32 size;
      stream >> size;
      quint8 flags;
      stream >> flags;
      connection->num_bytes = size > 0 ? size : -1;
      connection->flags = flags;
      if (flags & 32) {
        qDebug() << "PING!";
      }
      if (connection->num_bytes == -1) {
        return;
      }
    } else {
      return;
    }
  }

  if (socket->bytesAvailable() < connection->num_bytes) {
    return;
  }

  QByteArray data = socket->readAll();
  connection->num_bytes = -1;

  qDebug() << Q_FUNC_INFO;
  qDebug() << data;
  qDebug() << Q_FUNC_INFO << "done";

  if (connection->flags & 2) {
    QJson::Parser parser;
    bool ok = false;
    QVariant json = parser.parse(data, &ok);
    if (ok) {
      QVariantMap map = json.toMap();
      qDebug() << map;
    }
  }
}

void Remote::TomahawkDBDisconnected() {
  qDebug() << Q_FUNC_INFO;
}

void Remote::TomahawkDisconnected() {
  qDebug() << Q_FUNC_INFO;
}

void Remote::TomahawkError(QAbstractSocket::SocketError error) {
  qDebug() << Q_FUNC_INFO << error;
}

void Remote::TomahawkTransferConnected() {
  qDebug() << Q_FUNC_INFO;
  QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
  Q_ASSERT(socket);

  TomahawkConnection* connection = tomahawk_connections_[socket];

  {
    QVariantMap request;
    request["method"] = "accept-offer";
    request["key"] = connection->key;

    QJson::Serializer serialiser;
    const QByteArray& json = serialiser.serialize(request);

    QDataStream stream(socket);
    stream << (quint32)json.length();
    stream << (quint8)(2);
    stream.writeRawData(json.constData(), json.length());
  }

  {
    QDataStream stream(socket);
    stream << (quint32)2;
    stream << (quint8)(128 | 1);  // SETUP | RAW
    stream.writeRawData("ok", 2);
  }
}

void Remote::TomahawkTransferDisconnected() {
  qDebug() << Q_FUNC_INFO;
}

void Remote::TomahawkTransferReadyRead() {
  qDebug() << Q_FUNC_INFO;
}
