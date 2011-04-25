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

#include "messages.pb.h"
#include "spotifyclient.h"
#include "core/logging.h"

#include <QHostAddress>
#include <QTcpSocket>

#include <boost/scoped_array.hpp>

SpotifyClient::SpotifyClient(QObject* parent)
  : QObject(parent),
    socket_(new QTcpSocket(this))
{
  connect(socket_, SIGNAL(readyRead()), SLOT(SocketReadyRead()));
}

void SpotifyClient::Init(quint16 port) {
  qLog(Debug) << "port" << port;

  socket_->connectToHost(QHostAddress::LocalHost, port);
}

void SpotifyClient::SocketReadyRead() {
  QDataStream s(socket_);

  quint32 length = 0;
  s >> length;

  if (length == 0) {
    return;
  }

  boost::scoped_array<char> data(new char[length]);
  s.readRawData(data.get(), length);

  RequestMessage message;
  if (!message.ParseFromArray(data.get(), length)) {
    qLog(Error) << "Malformed protobuf message";
    socket_->deleteLater();
    socket_ = NULL;
    return;
  }

  qLog(Debug) << message.DebugString().c_str();

  if (message.has_login_request()) {
    const LoginRequest& r = message.login_request();
    emit Login(QString::fromUtf8(r.username().data(), r.username().length()),
               QString::fromUtf8(r.password().data(), r.password().length()));
  }
}

void SpotifyClient::SendMessage(const ResponseMessage& message) {
  qLog(Debug) << message.DebugString().c_str();

  std::string data(message.SerializeAsString());

  QDataStream s(socket_);
  s << quint32(data.length());
  s.writeRawData(data.data(), data.length());
}

void SpotifyClient::LoginCompleted(bool success, const QString& error) {
  const QByteArray error_bytes(error.toUtf8());

  ResponseMessage message;

  LoginResponse* response = message.mutable_login_response();
  response->set_success(success);
  response->set_error(error_bytes.constData(), error_bytes.length());

  SendMessage(message);
}
