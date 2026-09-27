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

#ifndef NETWORKREMOTE_INTERNETBROWSER_H_
#define NETWORKREMOTE_INTERNETBROWSER_H_

#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QModelIndexList>
#include <QObject>
#include <QPersistentModelIndex>
#include <QStringList>
#include <QTimer>
#include <functional>

#include "core/song.h"
#include "remotecontrolmessages.pb.h"

class InternetModel;
class QAbstractItemModel;
class QMimeData;

// Lets network remote clients browse the tree Clementine's Internet sidebar
// shows - every service and the nodes below it - and put nodes on the
// playlist as a drag from the sidebar does.
//
// Lives on the main thread with the models. The network remote's thread
// talks to it through queued signals carrying serialized messages and client
// ids, as with RendererRegistry.
//
// See the "Browsing Clementine's internet services from a remote" design.
class InternetBrowser : public QObject {
  Q_OBJECT

 public:
  // Most children sent in one BROWSE.
  static const int kMaxPageSize;
  // Node ids kept per client before older ones are forgotten.
  static const int kMaxIdsPerClient;

  // What the model's own data doesn't say about a node.
  struct ItemInfo {
    // LibraryItem::Type when the node belongs to a library model, else -1.
    int library_type = -1;
    // The song the node stands for, if any.
    Song song;
  };

  // How to find out about nodes. The defaults suit a plain model; HooksFor
  // gives the ones for Clementine's InternetModel.
  struct Hooks {
    // Whether the service a node belongs to has to be set up first.
    std::function<bool(const QModelIndex&)> needs_setup;
    std::function<ItemInfo(const QModelIndex&)> item_info;
    // Splits indexes, in order, into runs that come from the same source
    // model, each of which gets its own mimeData.
    std::function<QList<QModelIndexList>(const QModelIndexList&)>
        group_by_model;
  };
  static Hooks DefaultHooks();
  static Hooks HooksFor(InternetModel* model);

  explicit InternetBrowser(QObject* parent = nullptr);
  ~InternetBrowser();

  // The model to browse, normally InternetModel::merged_model(). Called on
  // the thread the model lives on; messages before then are ignored.
  void SetModel(QAbstractItemModel* model, const Hooks& hooks);

  // For tests.
  void SetTimeouts(int update_delay_msec, int loading_timeout_msec);

 public slots:
  // A serialized REQUEST_BROWSE or REQUEST_BROWSE_ADD from a client.
  void HandleMessage(int client_id, const QByteArray& data);
  void ClientDisconnected(int client_id);

 signals:
  // A serialized cpb::remote::Message for one client.
  void SendToClient(int client_id, const QByteArray& data);
  // As the sidebar's drag and drop does; the receiver takes ownership.
  void AddToPlaylist(QMimeData* data);

 private slots:
  void ModelChanged();
  void SendUpdates();

 private:
  struct Node {
    QPersistentModelIndex index;
    QString parent_id;  // Empty for the services.
  };

  struct Client {
    QHash<QString, Node> nodes;
    // Parent id -> the ids issued for its children.
    QHash<QString, QStringList> children;
    int next_id = 1;

    // The node last browsed, and the page asked for.
    bool watching = false;
    QString watched_id;
    int offset = 0;
    int limit = 0;
    // Children are expected (a node that loads on demand), so an empty list
    // is LOADING rather than READY until they come or loading_timeout_msec_.
    bool expecting_children = false;
    QElapsedTimer loading_since;
    QByteArray last_sent;
  };

  void Browse(int client_id, Client* client,
              const cpb::remote::RequestBrowse& request);
  void Add(int client_id, Client* client,
           const cpb::remote::RequestBrowseAdd& request);

  // Lists |client|'s watched node's children as a BROWSE message.
  QByteArray Describe(Client* client);
  // Everything about a node but its id.
  void DescribeNode(const QModelIndex& index, cpb::remote::BrowseNode* node);
  QString IdFor(Client* client, const QString& parent_id,
                const QModelIndex& index,
                QHash<QModelIndex, QString>* existing);
  QModelIndex Resolve(Client* client, const QString& id);
  void Prune(Client* client);
  QString ServiceName(const QModelIndex& index) const;
  QByteArray IconPng(const QModelIndex& index);
  void Send(int client_id, const cpb::remote::Message& msg);

  QAbstractItemModel* model_;
  Hooks hooks_;
  QHash<int, Client> clients_;
  QHash<QString, QByteArray> icons_;

  int update_delay_msec_;
  int loading_timeout_msec_;
  // Children, so they move to the main thread with the browser.
  QTimer* update_timer_;
  QTimer* loading_timer_;
};

#endif  // NETWORKREMOTE_INTERNETBROWSER_H_
