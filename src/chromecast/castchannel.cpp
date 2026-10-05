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

#include "castchannel.h"

#include <QJsonDocument>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QtEndian>
#include <cstring>

#include "cast_channel.pb.h"
#include "core/logging.h"

const char* CastChannel::kConnectionNamespace =
    "urn:x-cast:com.google.cast.tp.connection";
const char* CastChannel::kHeartbeatNamespace =
    "urn:x-cast:com.google.cast.tp.heartbeat";
const char* CastChannel::kReceiverId = "receiver-0";
const char* CastChannel::kSenderId = "sender-0";
const int CastChannel::kMaxMessageSize = 64 * 1024;

namespace {

using openscreen::cast::proto::CastMessage;

// Each message is preceded by its size.
const int kHeaderSize = sizeof(quint32_be);

const int kDefaultHeartbeatMsec = 5000;
// How many heartbeats may pass with nothing from the device.
const int kMissedHeartbeats = 3;
const int kConnectTimeoutMsec = 10000;

}  // namespace

QByteArray CastChannel::Encode(const Message& message) {
  CastMessage pb;
  pb.set_protocol_version(CastMessage::CASTV2_1_0);
  pb.set_source_id(message.source_id.toStdString());
  pb.set_destination_id(message.destination_id.toStdString());
  pb.set_namespace_(message.name_space.toStdString());
  pb.set_payload_type(CastMessage::STRING);
  pb.set_payload_utf8(message.payload.toStdString());

  const std::string data = pb.SerializeAsString();
  const quint32_be size(static_cast<quint32>(data.size()));
  QByteArray ret;
  ret.append(reinterpret_cast<const char*>(&size), sizeof(size));
  ret.append(data.data(), data.size());
  return ret;
}

bool CastChannel::Decode(QByteArray* buffer, QList<Message>* messages) {
  while (buffer->size() >= kHeaderSize) {
    quint32_be header;
    memcpy(&header, buffer->constData(), kHeaderSize);
    const quint32 size = header;
    if (size > static_cast<quint32>(kMaxMessageSize)) return false;
    if (buffer->size() < kHeaderSize + static_cast<int>(size)) break;

    CastMessage pb;
    if (!pb.ParseFromArray(buffer->constData() + kHeaderSize, size)) {
      return false;
    }
    buffer->remove(0, kHeaderSize + size);

    Message message;
    message.source_id = QString::fromStdString(pb.source_id());
    message.destination_id = QString::fromStdString(pb.destination_id());
    message.name_space = QString::fromStdString(pb.namespace_());
    if (pb.payload_type() == CastMessage::STRING) {
      message.payload = QString::fromStdString(pb.payload_utf8());
    }
    *messages << message;
  }
  return true;
}

CastChannel::CastChannel(QObject* parent)
    : QObject(parent), socket_(nullptr), open_(false) {
  heartbeat_.setInterval(kDefaultHeartbeatMsec);
  connect(&heartbeat_, SIGNAL(timeout()), SLOT(Heartbeat()));
}

CastChannel::~CastChannel() { Close(); }

void CastChannel::set_heartbeat_interval(int msec) {
  heartbeat_.setInterval(msec);
}

void CastChannel::Open(const QHostAddress& address, quint16 port) {
  Close();

  socket_ = new QSslSocket(this);
  QSslConfiguration config = socket_->sslConfiguration();
  // See the class comment.
  config.setPeerVerifyMode(QSslSocket::VerifyNone);
  socket_->setSslConfiguration(config);

  connect(socket_, SIGNAL(encrypted()), SLOT(Encrypted()));
  connect(socket_, SIGNAL(readyRead()), SLOT(ReadyRead()));
  connect(socket_, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
          SLOT(SocketError()));
  connect(socket_, SIGNAL(disconnected()), SLOT(SocketError()));

  // Heartbeat also gives up on a device that never finishes connecting.
  last_received_.start();
  heartbeat_.start();
  socket_->connectToHostEncrypted(address.toString(), port);
}

void CastChannel::Close() {
  heartbeat_.stop();
  open_ = false;
  buffer_.clear();
  connected_ids_.clear();
  if (!socket_) return;

  // Nothing from here on is news to whoever called Close.
  socket_->disconnect(this);
  if (socket_->state() == QAbstractSocket::ConnectedState &&
      socket_->isEncrypted()) {
    SendRaw(kConnectionNamespace, kReceiverId, {{"type", "CLOSE"}});
    socket_->flush();
  }
  socket_->abort();
  socket_->deleteLater();
  socket_ = nullptr;
}

void CastChannel::Fail(const QString& reason) {
  if (!socket_) return;
  qLog(Debug) << "Cast connection to" << socket_->peerName()
              << "ended:" << reason;
  Close();
  emit Closed(reason);
}

void CastChannel::Encrypted() {
  open_ = true;
  last_received_.start();
  SendRaw(kConnectionNamespace, kReceiverId,
          {{"type", "CONNECT"}, {"userAgent", "Clementine"}});
  connected_ids_ << kReceiverId;
  emit Opened();
}

void CastChannel::SocketError() {
  Fail(socket_->error() == QAbstractSocket::RemoteHostClosedError
           ? "The device closed the connection"
           : socket_->errorString());
}

void CastChannel::Heartbeat() {
  const qint64 limit = open_ ? qint64(heartbeat_.interval()) * kMissedHeartbeats
                             : kConnectTimeoutMsec;
  if (last_received_.elapsed() > limit) {
    Fail(open_ ? "The device stopped answering" : "Timed out connecting");
    return;
  }
  if (open_) SendRaw(kHeartbeatNamespace, kReceiverId, {{"type", "PING"}});
}

void CastChannel::Send(const QString& name_space, const QString& destination,
                       const QJsonObject& payload) {
  if (!open_) {
    qLog(Warning) << "Not sending to a Cast device that isn't connected";
    return;
  }
  if (!connected_ids_.contains(destination)) {
    SendRaw(kConnectionNamespace, destination,
            {{"type", "CONNECT"}, {"userAgent", "Clementine"}});
    connected_ids_ << destination;
  }
  SendRaw(name_space, destination, payload);
}

void CastChannel::SendRaw(const QString& name_space, const QString& destination,
                          const QJsonObject& payload) {
  Message message;
  message.source_id = kSenderId;
  message.destination_id = destination;
  message.name_space = name_space;
  message.payload =
      QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact));
  socket_->write(Encode(message));
}

void CastChannel::ReadyRead() {
  buffer_.append(socket_->readAll());
  last_received_.start();

  QList<Message> messages;
  const bool ok = Decode(&buffer_, &messages);
  for (const Message& message : messages) {
    Handle(message);
    // Handling a message can close the channel.
    if (!open_) return;
  }
  if (!ok) Fail("The device sent something that isn't a Cast message");
}

void CastChannel::Handle(const Message& message) {
  // Messages for other senders on the same connection aren't ours.
  if (message.destination_id != kSenderId && message.destination_id != "*") {
    return;
  }

  QJsonParseError error;
  const QJsonDocument doc =
      QJsonDocument::fromJson(message.payload.toUtf8(), &error);
  if (!doc.isObject()) {
    qLog(Debug) << "Ignoring a Cast message that isn't a JSON object in"
                << message.name_space;
    return;
  }
  const QJsonObject payload = doc.object();
  const QString type = payload["type"].toString();

  if (message.name_space == kHeartbeatNamespace) {
    if (type == "PING") {
      SendRaw(kHeartbeatNamespace, message.source_id, {{"type", "PONG"}});
    }
    return;
  }

  if (message.name_space == kConnectionNamespace && type == "CLOSE") {
    if (message.source_id == kReceiverId) {
      Fail("The device closed the connection");
      return;
    }
    // An app on the device went away; talking to it again needs a new
    // virtual connection.
    connected_ids_.remove(message.source_id);
  }

  emit MessageReceived(message.name_space, message.source_id, payload);
}
