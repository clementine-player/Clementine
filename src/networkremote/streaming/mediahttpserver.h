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

#ifndef NETWORKREMOTE_STREAMING_MEDIAHTTPSERVER_H_
#define NETWORKREMOTE_STREAMING_MEDIAHTTPSERVER_H_

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <memory>

class QTcpSocket;
class StreamItemTable;

// A parsed HTTP/1.x request head.
struct HttpRequest {
  QByteArray method;
  QByteArray path;
  QByteArray query;
  // Lower-case names.
  QMap<QByteArray, QByteArray> headers;

  // Parses everything up to (not including) the blank line.
  static bool Parse(const QByteArray& head, HttpRequest* request);
};

// A single byte range from a Range header.
struct ByteRange {
  enum Result {
    // No usable Range header: send the whole file.
    Whole,
    // Send [first, last].
    Partial,
    // 416 Range Not Satisfiable.
    Unsatisfiable,
  };

  static Result Parse(const QByteArray& header, qint64 size, qint64* first,
                      qint64* last);
};

// Serves media URLs (/s/<token>/<item>[?t=<ms>]) on sockets the network
// remote has recognised as HTTP. HTTP/1.1, one request per connection.
//
// See docs/design/remote-streaming.md section 5.
class MediaHttpServer : public QObject {
  Q_OBJECT

 public:
  explicit MediaHttpServer(std::shared_ptr<StreamItemTable> items,
                           QObject* parent = nullptr);
  ~MediaHttpServer();

  // Takes ownership of |socket|. It may already have data waiting.
  void HandleConnection(QTcpSocket* socket);

  static void WriteError(QTcpSocket* socket, int code, const char* reason);

  // Limits on what one renderer (one token), and everyone together, may have
  // open at once. Each pipeline is a decoder and an encoder with their own
  // threads, so those are limited more tightly.
  static const int kMaxResponsesPerRenderer;
  static const int kMaxPipelinesPerRenderer;
  static const int kMaxResponses;
  // Connections that are still sending their request.
  static const int kMaxPendingRequests;

 private slots:
  void ReadyRead();

 private:
  void Dispatch(QTcpSocket* socket, const HttpRequest& request);
  // Live responders, all of them or only those for |token|; pipelines only
  // with |pipelines|.
  int CountResponders(const QByteArray* token, bool pipelines) const;
  int CountPendingRequests() const;

  std::shared_ptr<StreamItemTable> items_;
};

#endif  // NETWORKREMOTE_STREAMING_MEDIAHTTPSERVER_H_
