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

#include <QDataStream>

RemoteClient::RemoteClient(Application* app, QTcpSocket* client)
  : app_(app),
    client_(client)
{
  // Open the buffer
  buffer_.setData(QByteArray());
  buffer_.open(QIODevice::ReadWrite);
  reading_protobuf_ = false;

  // Connect to the slot IncomingData when receiving data
  connect(client, SIGNAL(readyRead()), this, SLOT(IncomingData()));
}


RemoteClient::~RemoteClient() {
}

void RemoteClient::IncomingData() {
  while (client_->bytesAvailable()) {
    if (!reading_protobuf_) {
      // Read the length of the next message
      QDataStream s(client_);
      s >> expected_length_;
      reading_protobuf_ = true;
    }

    // Read some of the message
    buffer_.write(
      client_->read(expected_length_ - buffer_.size()));

    // Did we get everything?
    if (buffer_.size() == expected_length_) {
      // Parse the message
      emit Parse(buffer_.data());

      // Clear the buffer
      buffer_.close();
      buffer_.setData(QByteArray());
      buffer_.open(QIODevice::ReadWrite);
      reading_protobuf_ = false;
    }
  }
}

void RemoteClient::SendData(pb::remote::Message *msg) {
  // Serialize the message
  std::string data = msg->SerializeAsString();

  // write the length of the data first
  QDataStream s(client_);
  s << qint32(data.length());
  s.writeRawData(data.data(), data.length());

  // Flush data
  client_->flush();
}

QAbstractSocket::SocketState RemoteClient::State() {
  return client_->state();
}
