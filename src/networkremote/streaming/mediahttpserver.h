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
#include <QList>
#include <QMap>
#include <QObject>
#include <memory>

class QTcpSocket;
class EventCallbackTable;
class SleepInhibitor;
class StreamItemTable;
class StreamResponder;
struct StreamItem;

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

// A media URL's path: /<prefix>/<token>[/<item id>].
struct MediaPath {
  enum class Kind {
    // /s/<token>/<item id>: a renderer's stream.
    Stream,
    // /a/<token>/<item id>: the cover of a renderer's item or a remote's
    // synced song.
    Cover,
    // /d/<token>/<item id>: a song a remote downloads to sync.
    Download,
    // /e/<token>: a renderer's events, sent with NOTIFY.
    Event,
  };

  Kind kind = Kind::Stream;
  QByteArray token;
  // 0 for events.
  int item_id = 0;

  static bool Parse(const QByteArray& path, MediaPath* media_path);
};

// Serves media URLs on sockets the network remote has recognised as HTTP:
// /s/<token>/<item>[?t=<ms>] for renderers, /d/<token>/<item> for remotes
// downloading songs to sync, and /a/<token>/<item> for either's covers.
// HTTP/1.1, one request per connection.
//
// See docs/design/remote-streaming.md section 5.
class MediaHttpServer : public QObject {
  Q_OBJECT

 public:
  // Renderers' events are taken for |events|' tokens, if there is one.
  explicit MediaHttpServer(std::shared_ptr<StreamItemTable> items,
                           std::shared_ptr<EventCallbackTable> events = nullptr,
                           QObject* parent = nullptr);
  ~MediaHttpServer();

  // Where /d/ URLs, and synced songs' /a/ URLs, come from, with --media-sync.
  // Without, they're not found.
  void SetSyncItems(std::shared_ptr<StreamItemTable> items) {
    sync_items_ = items;
  }
  // Keeps the computer awake while a remote downloads from /d/, if set.
  void SetSleepInhibitor(SleepInhibitor* inhibitor) {
    sleep_inhibitor_ = inhibitor;
  }

  // Takes ownership of |socket|. It may already have data waiting.
  void HandleConnection(QTcpSocket* socket);

  static void WriteError(QTcpSocket* socket, int code, const char* reason);
  // DLNA's response headers for |item|, each ending in CRLF: for DLNA
  // renderers, and for any request that asks for them.
  static QByteArray DlnaHeaders(const StreamItem& item,
                                const HttpRequest& request);

  // Limits on what one renderer (one token), and everyone together, may have
  // open at once. Each pipeline is a decoder and an encoder with their own
  // threads, so those are limited more tightly.
  static const int kMaxResponsesPerRenderer;
  static const int kMaxPipelinesPerRenderer;
  static const int kMaxResponses;
  // Connections that are still sending their request.
  static const int kMaxPendingRequests;
  // The biggest event body taken. Sonos's grouping events can be tens of KB.
  static const int kMaxEventSize;
  // The biggest cover file served to a remote that syncs.
  static const int kMaxCoverFileSize;

 private slots:
  void ReadyRead();

 private:
  // Where a kind of path's items are found, in the order they're looked in,
  // and who may fetch them.
  struct ItemSource {
    enum class Access {
      // Whoever has the URL.
      Anyone,
      // Only the renderer or remote the item was offered to.
      OnlyItsPeer,
    };
    const StreamItemTable* table;
    Access access;
  };
  QList<ItemSource> Sources(MediaPath::Kind kind) const;

  void Dispatch(QTcpSocket* socket, const HttpRequest& request);
  // Sends a stream or a download, with a responder that owns the socket.
  void ServeItem(QTcpSocket* socket, const HttpRequest& request,
                 const MediaPath& path, const StreamItem& item, bool head_only);
  void WriteCover(QTcpSocket* socket, const StreamItem& item, bool head_only);
  // A NOTIFY to /e/<token>: reads its body, then hands it to the receiver.
  void ReadEvent(QTcpSocket* socket, const HttpRequest& request,
                 const QByteArray& token);
  // Live responders, all of them or only those for |token|; pipelines only
  // with |pipelines|.
  int CountResponders(const QByteArray* token, bool pipelines) const;
  int CountPendingRequests() const;
  void KeepAwake(StreamResponder* responder);

  std::shared_ptr<StreamItemTable> items_;
  std::shared_ptr<EventCallbackTable> events_;
  std::shared_ptr<StreamItemTable> sync_items_;
  SleepInhibitor* sleep_inhibitor_;
};

#endif  // NETWORKREMOTE_STREAMING_MEDIAHTTPSERVER_H_
