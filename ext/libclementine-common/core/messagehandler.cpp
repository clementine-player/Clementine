/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "messagehandler.h"

#include <QAbstractSocket>
#include <QDataStream>
#include <QLocalSocket>

#include "core/logging.h"

_MessageHandlerBase::_MessageHandlerBase(QIODevice* device, QObject* parent)
    : QObject(parent),
      device_(nullptr),
      flush_abstract_socket_(nullptr),
      flush_local_socket_(nullptr),
      reading_protobuf_(false),
      expected_length_(0),
      is_device_closed_(false) {
  if (device) {
    SetDevice(device);
  }
}

void _MessageHandlerBase::SetDevice(QIODevice* device) {
  device_ = device;

  buffer_.open(QIODevice::ReadWrite);

  connect(device, SIGNAL(readyRead()), SLOT(DeviceReadyRead()));

  // Yeah I know.
  if (QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(device)) {
    flush_abstract_socket_ = &QAbstractSocket::flush;
    connect(socket, SIGNAL(disconnected()), SLOT(DeviceClosed()));
  } else if (QLocalSocket* socket = qobject_cast<QLocalSocket*>(device)) {
    flush_local_socket_ = &QLocalSocket::flush;
    connect(socket, SIGNAL(disconnected()), SLOT(DeviceClosed()));
  } else {
    qFatal("Unsupported device type passed to _MessageHandlerBase");
  }
}

void _MessageHandlerBase::DeviceReadyRead() {
  // Cap incoming frames at 64 MiB to prevent corrupt or malicious length
  // prefixes from triggering unbounded allocations.
  static constexpr quint32 kMaxMessageLength = 64 * 1024 * 1024;

  while (device_->bytesAvailable()) {
    if (!reading_protobuf_) {
      // Check there are at least 4 bytes available to read expected_length_.
      // Otherwise a short read yields 0 and desynchronizes frame boundaries.
      if (device_->bytesAvailable() < static_cast<qint64>(sizeof(quint32))) {
        return;
      }

      // Read the length of the next message
      QDataStream s(device_);
      s >> expected_length_;
      // Reset framing state before closing so an unreadable or oversized
      // header leaves no stale partial frame on the stream.
      if (s.status() != QDataStream::Ok ||
          expected_length_ > kMaxMessageLength) {
        qLog(Error) << "Invalid protobuf message length:" << expected_length_;
        buffer_.close();
        buffer_.setData(QByteArray());
        reading_protobuf_ = false;
        device_->close();
        return;
      }

      reading_protobuf_ = true;
    }

    // Only read when payload bytes remain: a 0-byte message skips read(0)
    // and completes immediately, while an empty read waits for readyRead
    // instead of spinning.
    const qint64 remaining =
        static_cast<qint64>(expected_length_) - buffer_.size();
    if (remaining > 0) {
      const QByteArray chunk = device_->read(remaining);
      if (chunk.isEmpty()) {
        return;
      }
      buffer_.write(chunk);
    }

    // Did we get everything?
    if (buffer_.size() == static_cast<qint64>(expected_length_)) {
      // Parse the message
      if (!RawMessageArrived(buffer_.data())) {
        // Clear framing state before closing so corrupt protobuf payloads
        // cannot leave partial buffered data or desynchronize future reads.
        qLog(Error) << "Malformed protobuf message";
        buffer_.close();
        buffer_.setData(QByteArray());
        reading_protobuf_ = false;
        device_->close();
        return;
      }

      // Clear the buffer
      buffer_.close();
      buffer_.setData(QByteArray());
      buffer_.open(QIODevice::ReadWrite);
      reading_protobuf_ = false;
    }
  }
}

void _MessageHandlerBase::WriteMessage(const QByteArray& data) {
  QDataStream s(device_);
  s << quint32(data.length());
  s.writeRawData(data.data(), data.length());

  // Sorry.
  if (flush_abstract_socket_) {
    ((static_cast<QAbstractSocket*>(device_))->*(flush_abstract_socket_))();
  } else if (flush_local_socket_) {
    ((static_cast<QLocalSocket*>(device_))->*(flush_local_socket_))();
  }
}

void _MessageHandlerBase::DeviceClosed() {
  is_device_closed_ = true;
  AbortAll();
}
