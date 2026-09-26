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

#include "directresponder.h"

#include <QTcpSocket>

#include "core/logging.h"
#include "mediahttpserver.h"

namespace {
const qint64 kChunkSize = 64 * 1024;
// Keep this much queued in the socket and no more, so a whole file is never
// read into memory.
const qint64 kHighWater = 256 * 1024;
}  // namespace

DirectResponder::DirectResponder(QTcpSocket* socket, const QByteArray& token,
                                 const StreamItem& item,
                                 const QByteArray& range, bool head_only,
                                 QObject* parent)
    : StreamResponder(socket, token, Direct, parent),
      file_(item.req.MediaUrl().toLocalFile()),
      remaining_(0) {
  if (!file_.open(QIODevice::ReadOnly)) {
    qLog(Warning) << "Couldn't open" << file_.fileName() << "to stream";
    MediaHttpServer::WriteError(socket_, 404, "Not Found");
    return;
  }

  const qint64 size = file_.size();
  qint64 first = 0;
  qint64 last = size - 1;
  const ByteRange::Result result = ByteRange::Parse(range, size, &first, &last);

  if (result == ByteRange::Unsatisfiable) {
    socket_->write(QString("HTTP/1.1 416 Range Not Satisfiable\r\n"
                           "Content-Range: bytes */%1\r\n"
                           "Content-Length: 0\r\n"
                           "Connection: close\r\n\r\n")
                       .arg(size)
                       .toLatin1());
    socket_->disconnectFromHost();
    return;
  }

  QByteArray head;
  if (result == ByteRange::Partial) {
    head += "HTTP/1.1 206 Partial Content\r\n";
    head += QString("Content-Range: bytes %1-%2/%3\r\n")
                .arg(first)
                .arg(last)
                .arg(size)
                .toLatin1();
  } else {
    head += "HTTP/1.1 200 OK\r\n";
  }
  head += "Content-Type: " + item.plan.mime_type.toLatin1() + "\r\n";
  head += "Content-Length: " + QByteArray::number(last - first + 1) + "\r\n";
  head += "Accept-Ranges: bytes\r\n";
  head += "Cache-Control: no-store\r\n";
  head += "Connection: close\r\n\r\n";
  socket_->write(head);

  if (head_only) {
    socket_->disconnectFromHost();
    return;
  }

  file_.seek(first);
  remaining_ = last - first + 1;
  connect(socket_, SIGNAL(bytesWritten(qint64)), SLOT(Pump()));
  Pump();
}

DirectResponder::~DirectResponder() {}

void DirectResponder::Pump() {
  while (remaining_ > 0 && socket_->bytesToWrite() < kHighWater) {
    const QByteArray data = file_.read(qMin(kChunkSize, remaining_));
    if (data.isEmpty()) {
      // The file got shorter under us.
      socket_->abort();
      return;
    }
    socket_->write(data);
    remaining_ -= data.size();
  }

  if (remaining_ == 0) {
    disconnect(socket_, SIGNAL(bytesWritten(qint64)), this, SLOT(Pump()));
    // Sends what's still queued, then closes.
    socket_->disconnectFromHost();
  }
}
