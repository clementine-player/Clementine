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

#include "mediahttpserver.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>

#include "core/logging.h"
#include "core/timeconstants.h"
#include "directresponder.h"
#include "eventcallbacktable.h"
#include "pipelineresponder.h"
#include "streamitemtable.h"

const int MediaHttpServer::kMaxResponsesPerRenderer = 4;
const int MediaHttpServer::kMaxPipelinesPerRenderer = 2;
const int MediaHttpServer::kMaxResponses = 32;
const int MediaHttpServer::kMaxPendingRequests = 32;
const int MediaHttpServer::kMaxEventSize = 512 * 1024;

namespace {

const int kMaxHeadSize = 8 * 1024;
const int kHeadTimeoutMsec = 10000;
const char* kHandledProperty = "clementine_http_handled";

}  // namespace

bool HttpRequest::Parse(const QByteArray& head, HttpRequest* request) {
  const QList<QByteArray> lines = head.split('\n');
  if (lines.isEmpty()) return false;

  const QList<QByteArray> parts = lines[0].trimmed().split(' ');
  if (parts.size() != 3 || !parts[2].startsWith("HTTP/1.")) return false;

  request->method = parts[0];
  const QByteArray target = parts[1];
  const int question = target.indexOf('?');
  request->path = question == -1 ? target : target.left(question);
  request->query = question == -1 ? QByteArray() : target.mid(question + 1);

  for (int i = 1; i < lines.size(); ++i) {
    const QByteArray line = lines[i].trimmed();
    if (line.isEmpty()) continue;
    const int colon = line.indexOf(':');
    if (colon <= 0) return false;
    request->headers[line.left(colon).trimmed().toLower()] =
        line.mid(colon + 1).trimmed();
  }
  return true;
}

ByteRange::Result ByteRange::Parse(const QByteArray& header, qint64 size,
                                   qint64* first, qint64* last) {
  if (!header.startsWith("bytes=")) return Whole;
  const QByteArray spec = header.mid(6).trimmed();
  // Multiple ranges are allowed to be answered with the whole file.
  if (spec.contains(',')) return Whole;

  const int dash = spec.indexOf('-');
  if (dash == -1) return Whole;
  const QByteArray start = spec.left(dash).trimmed();
  const QByteArray end = spec.mid(dash + 1).trimmed();

  bool ok = true;
  if (start.isEmpty()) {
    // bytes=-N: the last N bytes.
    const qint64 suffix = end.toLongLong(&ok);
    if (!ok || suffix <= 0) return Unsatisfiable;
    *first = qMax(0ll, size - suffix);
    *last = size - 1;
  } else {
    *first = start.toLongLong(&ok);
    if (!ok) return Whole;
    *last = end.isEmpty() ? size - 1 : end.toLongLong(&ok);
    if (!ok) return Whole;
    *last = qMin(*last, size - 1);
  }

  if (size == 0 || *first >= size || *first > *last) return Unsatisfiable;
  return Partial;
}

MediaHttpServer::MediaHttpServer(std::shared_ptr<StreamItemTable> items,
                                 std::shared_ptr<EventCallbackTable> events,
                                 QObject* parent)
    : QObject(parent), items_(items), events_(events) {}

// Responders are children, so they're deleted here too: streams stop when the
// server does, for example when streaming is turned off.
MediaHttpServer::~MediaHttpServer() {}

int MediaHttpServer::CountResponders(const QByteArray* token,
                                     bool pipelines) const {
  int count = 0;
  for (const StreamResponder* responder :
       findChildren<StreamResponder*>(QString(), Qt::FindDirectChildrenOnly)) {
    if (!responder->active()) continue;
    if (token && responder->token() != *token) continue;
    if (pipelines && responder->kind() != StreamResponder::Pipeline) continue;
    ++count;
  }
  return count;
}

int MediaHttpServer::CountPendingRequests() const {
  int count = 0;
  for (const QTcpSocket* socket :
       findChildren<QTcpSocket*>(QString(), Qt::FindDirectChildrenOnly)) {
    if (!socket->property(kHandledProperty).toBool()) ++count;
  }
  return count;
}

void MediaHttpServer::HandleConnection(QTcpSocket* socket) {
  if (CountPendingRequests() >= kMaxPendingRequests) {
    qLog(Warning) << "Too many incomplete media requests; dropping one from"
                  << socket->peerAddress().toString();
    socket->abort();
    socket->deleteLater();
    return;
  }

  socket->setParent(this);
  connect(socket, SIGNAL(readyRead()), SLOT(ReadyRead()));
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

  // Don't let a client hold a socket open without finishing its request.
  QTimer::singleShot(kHeadTimeoutMsec, socket, [socket]() {
    if (!socket->property(kHandledProperty).toBool()) socket->abort();
  });

  // The request may already be buffered: the network remote peeked at it to
  // tell it was HTTP.
  QMetaObject::invokeMethod(this, "ReadyRead", Qt::QueuedConnection);
}

void MediaHttpServer::ReadyRead() {
  // Called for one socket from readyRead(), or for every pending socket after
  // HandleConnection.
  QList<QTcpSocket*> sockets;
  if (QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender())) {
    sockets << socket;
  } else {
    sockets = findChildren<QTcpSocket*>(QString(), Qt::FindDirectChildrenOnly);
  }

  for (QTcpSocket* socket : sockets) {
    if (socket->property(kHandledProperty).toBool()) continue;

    const QByteArray peeked = socket->peek(kMaxHeadSize + 4);
    const int end = peeked.indexOf("\r\n\r\n");
    if (end == -1) {
      if (peeked.size() > kMaxHeadSize) {
        // Answer once, however much more the client sends.
        socket->setProperty(kHandledProperty, true);
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(ReadyRead()));
        WriteError(socket, 431, "Request Header Fields Too Large");
      }
      continue;
    }

    const QByteArray head = socket->read(end + 4).left(end);
    socket->setProperty(kHandledProperty, true);
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(ReadyRead()));

    HttpRequest request;
    if (!HttpRequest::Parse(head, &request)) {
      WriteError(socket, 400, "Bad Request");
      continue;
    }
    Dispatch(socket, request);
  }
}

void MediaHttpServer::Dispatch(QTcpSocket* socket, const HttpRequest& request) {
  if (request.method == "NOTIFY") {
    ReadEvent(socket, request);
    return;
  }

  const bool head_only = request.method == "HEAD";
  if (request.method != "GET" && !head_only) {
    WriteError(socket, 405, "Method Not Allowed");
    return;
  }

  // /s/<token>/<item id>, or its cover at /a/<token>/<item id>
  const QList<QByteArray> parts = request.path.split('/');
  bool id_ok = false;
  const int item_id = parts.size() == 4 ? parts[3].toInt(&id_ok) : 0;
  if (parts.size() != 4 || !parts[0].isEmpty() ||
      (parts[1] != "s" && parts[1] != "a") || !id_ok) {
    WriteError(socket, 404, "Not Found");
    return;
  }

  StreamItem item;
  QHostAddress allowed_peer;
  if (!items_->Find(parts[2], item_id, &item, &allowed_peer)) {
    WriteError(socket, 404, "Not Found");
    return;
  }

  // Covers are shown by whatever controls the renderer too, such as the
  // Sonos app, which fetches them itself. Their URL is enough.
  if (parts[1] == "a") {
    WriteCover(socket, item, head_only);
    return;
  }

  // The URL is a capability, but only for the renderer it was given to.
  if (NormalisedAddress(socket->peerAddress()) !=
      NormalisedAddress(allowed_peer)) {
    qLog(Warning) << "Media request from" << socket->peerAddress()
                  << "for a renderer at" << allowed_peer;
    WriteError(socket, 403, "Forbidden");
    return;
  }

  const QByteArray& token = parts[2];
  const bool pipeline = item.plan.mode != StreamPlan::Direct;
  if (CountResponders(nullptr, false) >= kMaxResponses) {
    WriteError(socket, 503, "Service Unavailable");
    return;
  }
  if (CountResponders(&token, false) >= kMaxResponsesPerRenderer ||
      (pipeline && CountResponders(&token, true) >= kMaxPipelinesPerRenderer)) {
    qLog(Warning) << "Too many media requests from" << socket->peerAddress();
    WriteError(socket, 429, "Too Many Requests");
    return;
  }

  qLog(Debug) << "Serving item" << item_id << request.method
              << request.headers.value("range");

  // The responder owns the socket from here and deletes itself with it.
  disconnect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

  if (!pipeline) {
    new DirectResponder(socket, token, item, request.headers.value("range"),
                        head_only, this);
    return;
  }

  // Where to start, for seeking: within the item, or at its start if it has
  // no known length, such as radio.
  bool ok = false;
  qint64 start_ms = QUrlQuery(QString::fromLatin1(request.query))
                        .queryItemValue("t")
                        .toLongLong(&ok);
  const qint64 length_ms = item.plan.length_nanosec / kNsecPerMsec;
  if (!ok || start_ms < 0 || length_ms <= 0) start_ms = 0;
  start_ms = qMin(start_ms, length_ms > 0 ? length_ms : 0);

  new PipelineResponder(socket, token, item, start_ms, head_only, this);
}

void MediaHttpServer::ReadEvent(QTcpSocket* socket,
                                const HttpRequest& request) {
  // /e/<token>
  const QList<QByteArray> parts = request.path.split('/');
  QHostAddress allowed_peer;
  if (!events_ || parts.size() != 3 || !parts[0].isEmpty() || parts[1] != "e" ||
      !events_->Find(parts[2], &allowed_peer)) {
    WriteError(socket, 404, "Not Found");
    return;
  }
  if (NormalisedAddress(socket->peerAddress()) !=
      NormalisedAddress(allowed_peer)) {
    qLog(Warning) << "Event from" << socket->peerAddress()
                  << "for a renderer at" << allowed_peer;
    WriteError(socket, 403, "Forbidden");
    return;
  }

  bool ok = false;
  const qint64 length = request.headers.value("content-length").toLongLong(&ok);
  if (!ok || length < 0) {
    WriteError(socket, 411, "Length Required");
    return;
  }
  if (length > kMaxEventSize) {
    WriteError(socket, 413, "Content Too Large");
    return;
  }

  // The body may still be on its way.
  const QByteArray token = parts[2];
  auto body = std::make_shared<QByteArray>();
  auto connection = std::make_shared<QMetaObject::Connection>();
  auto read = [this, socket, token, length, body, connection]() {
    *body += socket->read(length - body->size());
    if (body->size() < length) return;
    disconnect(*connection);

    if (!events_->Deliver(token, *body)) {
      WriteError(socket, 404, "Not Found");
      return;
    }
    socket->write(
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n");
    socket->disconnectFromHost();
  };
  *connection = connect(socket, &QTcpSocket::readyRead, this, read);
  QTimer::singleShot(kHeadTimeoutMsec, socket, [socket, body, length]() {
    if (body->size() < length) socket->abort();
  });
  read();
}

void MediaHttpServer::WriteCover(QTcpSocket* socket, const StreamItem& item,
                                 bool head_only) {
  qLog(Debug) << "Serving the cover of item" << item.id << "to"
              << socket->peerAddress() << "," << item.cover.size() << "bytes";
  if (item.cover.isEmpty()) {
    WriteError(socket, 404, "Not Found");
    return;
  }
  QByteArray head = "HTTP/1.1 200 OK\r\n";
  head += "Content-Type: image/jpeg\r\n";
  head += "Content-Length: " + QByteArray::number(item.cover.size()) + "\r\n";
  head += "Cache-Control: no-store\r\n";
  head += "Connection: close\r\n\r\n";
  socket->write(head);
  if (!head_only) socket->write(item.cover);
  socket->disconnectFromHost();
}

void MediaHttpServer::WriteError(QTcpSocket* socket, int code,
                                 const char* reason) {
  const QByteArray body = QByteArray(reason) + "\n";
  socket->write(QString("HTTP/1.1 %1 %2\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %3\r\n"
                        "Connection: close\r\n\r\n")
                    .arg(code)
                    .arg(reason)
                    .arg(body.size())
                    .toLatin1());
  socket->write(body);
  socket->disconnectFromHost();
}
