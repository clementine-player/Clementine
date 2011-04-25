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

#include "spotifyblob/messages.pb.h"

#include <QTcpServer>
#include <QTcpSocket>

#include <boost/scoped_array.hpp>

SpotifyServer::SpotifyServer(QObject* parent)
  : QObject(parent),
    server_(new QTcpServer(this)),
    protocol_socket_(NULL)
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

  emit ClientConnected();
}

void SpotifyServer::Login(const QString& username, const QString& password) {
  const QByteArray username_bytes(username.toUtf8());
  const QByteArray password_bytes(password.toUtf8());

  RequestMessage message;

  LoginRequest* request = message.mutable_login_request();
  request->set_username(username_bytes.constData(), username_bytes.length());
  request->set_password(password_bytes.constData(), password_bytes.length());

  SendMessage(message);
}

void SpotifyServer::SendMessage(const RequestMessage& message) {
  qLog(Debug) << message.DebugString().c_str();

  std::string data(message.SerializeAsString());

  QDataStream s(protocol_socket_);
  s << quint32(data.length());
  s.writeRawData(data.data(), data.length());
}

void SpotifyServer::ProtocolSocketReadyRead() {
  QDataStream s(protocol_socket_);

  quint32 length = 0;
  s >> length;

  if (length == 0) {
    return;
  }

  boost::scoped_array<char> data(new char[length]);
  s.readRawData(data.get(), length);

  ResponseMessage message;
  if (!message.ParseFromArray(data.get(), length)) {
    qLog(Error) << "Malformed protobuf message";
    protocol_socket_->deleteLater();
    protocol_socket_ = NULL;
    return;
  }

  qLog(Debug) << message.DebugString().c_str();

  if (message.has_login_response()) {
    const LoginResponse& response = message.login_response();
    if (!response.success()) {
      qLog(Info) << QString::fromUtf8(response.error().data(), response.error().size());
    }
    emit LoginCompleted(response.success());
  }
}
