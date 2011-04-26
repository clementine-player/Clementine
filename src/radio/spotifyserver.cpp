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

#include "spotifyserver.h"
#include "core/logging.h"

#include "spotifyblob/spotifymessages.pb.h"

#include <QTcpServer>
#include <QTcpSocket>

SpotifyServer::SpotifyServer(QObject* parent)
  : QObject(parent),
    server_(new QTcpServer(this)),
    protocol_socket_(NULL),
    logged_in_(false)
{
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
}

void SpotifyServer::Init() {
  if (!server_->listen(QHostAddress::LocalHost)) {
    qLog(Error) << "Couldn't open server socket" << server_->errorString();
  }
}

int SpotifyServer::server_port() const {
  return server_->serverPort();
}

void SpotifyServer::NewConnection() {
  protocol_socket_ = server_->nextPendingConnection();
  connect(protocol_socket_, SIGNAL(readyRead()), SLOT(ProtocolSocketReadyRead()));

  qLog(Info) << "Connection from port" << protocol_socket_->peerPort();

  // Send any login messages that were queued before the client connected
  foreach (const protobuf::SpotifyMessage& message, queued_login_messages_) {
    SendMessage(message);
  }
  queued_login_messages_.clear();
}

void SpotifyServer::SendMessage(const protobuf::SpotifyMessage& message) {
  const bool is_login_message = message.has_login_request();

  QList<protobuf::SpotifyMessage>* queue =
      is_login_message ? &queued_login_messages_ : &queued_messages_;

  if (!protocol_socket_ || (!is_login_message && !logged_in_)) {
    queue->append(message);
  } else {
    SpotifyMessageUtils::SendMessage(protocol_socket_, message);
  }
}

void SpotifyServer::Login(const QString& username, const QString& password) {
  protobuf::SpotifyMessage message;

  protobuf::LoginRequest* request = message.mutable_login_request();
  request->set_username(DataCommaSizeFromQString(username));
  request->set_password(DataCommaSizeFromQString(password));

  SendMessage(message);
}

void SpotifyServer::ProtocolSocketReadyRead() {
  protobuf::SpotifyMessage message;
  if (!ReadMessage(protocol_socket_, &message)) {
    protocol_socket_->deleteLater();
    protocol_socket_ = NULL;
    return;
  }

  if (message.has_login_response()) {
    const protobuf::LoginResponse& response = message.login_response();
    logged_in_ = response.success();

    if (!response.success()) {
      qLog(Info) << QStringFromStdString(response.error());
    } else {
      // Send any messages that were queued before the client logged in
      foreach (const protobuf::SpotifyMessage& message, queued_messages_) {
        SendMessage(message);
      }
      queued_messages_.clear();
    }

    emit LoginCompleted(response.success());
  } else if (message.has_playlists_updated()) {
    emit PlaylistsUpdated(message.playlists_updated());
  }
}
