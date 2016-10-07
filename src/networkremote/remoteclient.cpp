/* This file is part of Clementine.
   Copyright 2013, Andreas Muttscheller <asfa194@gmail.com>

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

#include "core/logging.h"

#include "remoteclient.h"
#include "networkremote.h"

#include <QDataStream>
#include <QSettings>

RemoteClient::RemoteClient(Application* app, QTcpSocket* client)
    : app_(app),
      downloader_(false),
      client_(client),
      song_sender_(new SongSender(app, this)) {
  // Open the buffer
  buffer_.setData(QByteArray());
  buffer_.open(QIODevice::ReadWrite);
  reading_protobuf_ = false;

  // Connect to the slot IncomingData when receiving data
  connect(client, SIGNAL(readyRead()), this, SLOT(IncomingData()));

  // Check if we use auth code
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  use_auth_code_ = s.value("use_auth_code", false).toBool();
  auth_code_ = s.value("auth_code", 0).toInt();
  allow_downloads_ = s.value("allow_downloads", false).toBool();

  s.endGroup();

  // If we don't use an auth code, we don't need to authenticate the client.
  authenticated_ = !use_auth_code_;
}

RemoteClient::~RemoteClient() {
  client_->close();
  if (client_->state() == QAbstractSocket::ConnectedState)
    client_->waitForDisconnected(2000);

  song_sender_->deleteLater();
  client_->deleteLater();
}

void RemoteClient::setDownloader(bool downloader) { downloader_ = downloader; }

void RemoteClient::IncomingData() {
  while (client_->bytesAvailable()) {
    if (!reading_protobuf_) {
      // If we have less than 4 byte, we cannot read the length. Wait for more data
      if (client_->bytesAvailable() < 4) {
        break;
      }
      // Read the length of the next message
      QDataStream s(client_);
      s >> expected_length_;

      // Receiving more than 128mb is very unlikely
      // Flush the data and disconnect the client
      if (expected_length_ > 134217728) {
        qLog(Debug) << "Received invalid data, disconnect client";
        qLog(Debug) << "expected_length_ =" << expected_length_;
        client_->close();
        return;
      }

      reading_protobuf_ = true;
    }

    // Read some of the message
    buffer_.write(client_->read(expected_length_ - buffer_.size()));

    // Did we get everything?
    if (buffer_.size() == expected_length_) {
      // Parse the message
      ParseMessage(buffer_.data());

      // Clear the buffer
      buffer_.close();
      buffer_.setData(QByteArray());
      buffer_.open(QIODevice::ReadWrite);
      reading_protobuf_ = false;
    }
  }
}

void RemoteClient::ParseMessage(const QByteArray& data) {
  pb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) {
    qLog(Info) << "Couldn't parse data";
    return;
  }

  if (msg.type() == pb::remote::CONNECT && use_auth_code_) {
    if (msg.request_connect().auth_code() != auth_code_) {
      DisconnectClient(pb::remote::Wrong_Auth_Code);
      return;
    } else {
      authenticated_ = true;
    }
  }

  if (msg.type() == pb::remote::CONNECT) {
    setDownloader(msg.request_connect().downloader());
    qDebug() << "Downloader" << downloader_;
  }

  // Check if downloads are allowed
  if (msg.type() == pb::remote::DOWNLOAD_SONGS && !allow_downloads_) {
    DisconnectClient(pb::remote::Download_Forbidden);
    return;
  }

  if (msg.type() == pb::remote::DISCONNECT) {
    client_->abort();
    qDebug() << "Client disconnected";
    return;
  }

  // Check if the client has sent the correct auth code
  if (!authenticated_) {
    DisconnectClient(pb::remote::Not_Authenticated);
    return;
  }

  // Now parse the other data
  emit Parse(msg);
}

void RemoteClient::DisconnectClient(pb::remote::ReasonDisconnect reason) {
  pb::remote::Message msg;
  msg.set_type(pb::remote::DISCONNECT);

  msg.mutable_response_disconnect()->set_reason_disconnect(reason);
  SendDataToClient(&msg);

  // Just close the connection. The next time the outgoing data creator
  // sends a keep alive, the client will be deleted
  client_->close();
}

// Sends data to client without check if authenticated
void RemoteClient::SendDataToClient(pb::remote::Message* msg) {
  // Set the default version
  msg->set_version(msg->default_instance().version());

  // Check if we are still connected
  if (client_->state() == QTcpSocket::ConnectedState) {
    // Serialize the message
    std::string data = msg->SerializeAsString();

    // write the length of the data first
    QDataStream s(client_);
    s << qint32(data.length());
    if (downloader_) {
      // Don't use QDataSteam for large files
      client_->write(data.data(), data.length());
    } else {
      s.writeRawData(data.data(), data.length());
    }

    // Do NOT flush data here! If the client is already disconnected, it
    // causes a SIGPIPE termination!!!
  } else {
    qDebug() << "Closed";
    client_->close();
  }
}

void RemoteClient::SendData(pb::remote::Message* msg) {
  // Check if client is authenticated before sending the data
  if (authenticated_) {
    SendDataToClient(msg);
  }
}

QAbstractSocket::SocketState RemoteClient::State() { return client_->state(); }
