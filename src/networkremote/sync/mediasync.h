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

#ifndef NETWORKREMOTE_SYNC_MEDIASYNC_H_
#define NETWORKREMOTE_SYNC_MEDIASYNC_H_

#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <functional>
#include <memory>

#include "networkremote/streaming/streamitemtable.h"
#include "networkremote/sync/syncresolver.h"
#include "remotecontrolmessages.pb.h"

class LibraryBackend;
class UrlHandler;

// Answers remotes' sync requests (with --media-sync): lists the sources a
// remote can keep, and resolves a rule's source into a manifest of songs, each
// with a /d/ URL that only that remote may fetch. Keeps no state about rules,
// only what each connected remote may download. Lives on the main thread, like
// InternetBrowser, and talks to the network remote through queued signals
// carrying serialized messages.
class MediaSync : public QObject {
  Q_OBJECT

 public:
  struct Hooks {
    SyncResolver::Playlists playlists;
    SyncResolver::SmartPlaylists smart_playlists;
    // The playlists and smart playlists a remote can keep, for
    // REQUEST_SYNC_SOURCES.
    std::function<QList<cpb::remote::SyncSource>()> sources;
    // The UrlHandler for a song's URL scheme, if any.
    std::function<const UrlHandler*(const Song&)> url_handler;
    // Whether the network remote's settings let remotes download songs.
    std::function<bool()> downloads_allowed;
  };

  // The most songs one remote may have on offer at once, across its
  // manifests.
  static const int kMaxItemsPerClient;

  MediaSync(LibraryBackend* library, const Hooks& hooks,
            std::shared_ptr<StreamItemTable> items, QObject* parent = nullptr);
  ~MediaSync();

  // What the HTTP server serves /d/ URLs from.
  std::shared_ptr<StreamItemTable> items() const { return items_; }

 public slots:
  void HandleMessage(int client_id, const QString& peer_address,
                     const QByteArray& data);
  void ClientDisconnected(int client_id);

 signals:
  void SendToClient(int client_id, const QByteArray& data);

 private:
  struct Client {
    QByteArray token;
    QHostAddress peer;
    QList<StreamItem> items;
    // Item keys and versions on offer, to their ids.
    QHash<QString, int> ids;
  };

  void Manifest(Client* client, const cpb::remote::RequestSyncManifest& req,
                cpb::remote::ResponseSyncManifest* response);
  // The id of |item| in |client|'s offer, adding it if need be. 0 when the
  // offer is full.
  int Offer(Client* client, const QString& key, const StreamItem& item);

  LibraryBackend* library_;
  Hooks hooks_;
  std::shared_ptr<StreamItemTable> items_;
  QHash<int, Client> clients_;
};

#endif  // NETWORKREMOTE_SYNC_MEDIASYNC_H_
