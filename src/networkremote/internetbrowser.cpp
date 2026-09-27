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

#include "internetbrowser.h"

#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <QBuffer>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QPixmap>

#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/mimedata.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"
#include "library/libraryitem.h"
#include "library/librarymodel.h"
#include "networkremote/outgoingdatacreator.h"

const int InternetBrowser::kMaxPageSize = 500;
const int InternetBrowser::kMaxIdsPerClient = 20000;

namespace {

const int kUpdateDelayMsec = 250;
const int kLoadingTimeoutMsec = 30000;
const int kIconSize = 48;

}  // namespace

InternetBrowser::Hooks InternetBrowser::DefaultHooks() {
  Hooks hooks;
  hooks.needs_setup = [](const QModelIndex&) { return false; };
  hooks.item_info = [](const QModelIndex& index) {
    ItemInfo info;
    info.song = index.data(InternetModel::Role_SongMetadata).value<Song>();
    return info;
  };
  hooks.group_by_model = [](const QModelIndexList& indexes) {
    return QList<QModelIndexList>{indexes};
  };
  return hooks;
}

InternetBrowser::Hooks InternetBrowser::HooksFor(InternetModel* model) {
  Hooks hooks = DefaultHooks();

  hooks.needs_setup = [model](const QModelIndex& index) {
    InternetService* service = model->ServiceForIndex(index);
    return service && service->NeedsSetup();
  };

  // Services that have a library of their own (Subsonic, Plex, the cloud
  // drives...) graft a LibraryModel into the tree, whose items keep their
  // song to themselves.
  hooks.item_info = [](const QModelIndex& index) {
    QModelIndex source = index;
    while (const QAbstractProxyModel* proxy =
               qobject_cast<const QAbstractProxyModel*>(source.model())) {
      source = proxy->mapToSource(source);
    }
    ItemInfo info;
    if (const LibraryModel* library =
            qobject_cast<const LibraryModel*>(source.model())) {
      if (LibraryItem* item = library->IndexToItem(source)) {
        info.library_type = item->type;
        if (item->type == LibraryItem::Type_Song) info.song = item->metadata;
      }
      return info;
    }
    info.song = index.data(InternetModel::Role_SongMetadata).value<Song>();
    return info;
  };

  // MergedProxyModel::mimeData only asks the first index's model, and drops
  // the indexes that belong to any other.
  hooks.group_by_model = [](const QModelIndexList& indexes) {
    QList<QModelIndexList> groups;
    const QAbstractItemModel* last = nullptr;
    for (const QModelIndex& index : indexes) {
      const MergedProxyModel* merged =
          qobject_cast<const MergedProxyModel*>(index.model());
      const QAbstractItemModel* source =
          merged ? merged->mapToSource(index).model() : index.model();
      if (groups.isEmpty() || source != last) groups << QModelIndexList();
      groups.last() << index;
      last = source;
    }
    return groups;
  };

  return hooks;
}

InternetBrowser::InternetBrowser(QObject* parent)
    : QObject(parent),
      model_(nullptr),
      hooks_(DefaultHooks()),
      update_delay_msec_(kUpdateDelayMsec),
      loading_timeout_msec_(kLoadingTimeoutMsec) {
  update_timer_.setSingleShot(true);
  loading_timer_.setSingleShot(true);
  connect(&update_timer_, SIGNAL(timeout()), SLOT(SendUpdates()));
  connect(&loading_timer_, SIGNAL(timeout()), SLOT(SendUpdates()));
}

InternetBrowser::~InternetBrowser() {}

void InternetBrowser::SetModel(QAbstractItemModel* model, const Hooks& hooks) {
  model_ = model;
  hooks_ = hooks;

  // Any change could be under a node a client is looking at. Rather than
  // work out which, the watched nodes are listed again after a short delay
  // and sent only if they changed.
  for (const char* signal :
       {SIGNAL(rowsInserted(QModelIndex, int, int)),
        SIGNAL(rowsRemoved(QModelIndex, int, int)),
        SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
        SIGNAL(dataChanged(QModelIndex, QModelIndex, QList<int>)),
        SIGNAL(layoutChanged(QList<QPersistentModelIndex>,
                             QAbstractItemModel::LayoutChangeHint)),
        SIGNAL(modelReset())}) {
    connect(model_, signal, SLOT(ModelChanged()));
  }
}

void InternetBrowser::SetTimeouts(int update_delay_msec,
                                  int loading_timeout_msec) {
  update_delay_msec_ = update_delay_msec;
  loading_timeout_msec_ = loading_timeout_msec;
}

void InternetBrowser::HandleMessage(int client_id, const QByteArray& data) {
  if (!model_) return;

  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;

  Client* client = &clients_[client_id];
  switch (msg.type()) {
    case cpb::remote::REQUEST_BROWSE:
      Browse(client_id, client, msg.request_browse());
      break;
    case cpb::remote::REQUEST_BROWSE_ADD:
      Add(client_id, client, msg.request_browse_add());
      break;
    default:
      break;
  }
}

void InternetBrowser::ClientDisconnected(int client_id) {
  clients_.remove(client_id);
}

void InternetBrowser::Browse(int client_id, Client* client,
                             const cpb::remote::RequestBrowse& request) {
  client->watching = true;
  client->watched_id = QString::fromStdString(request.node_id());
  client->offset = qMax(0, request.offset());
  client->limit =
      request.limit() > 0 ? qMin(request.limit(), kMaxPageSize) : kMaxPageSize;

  // A node that loads its children on demand is expected to get some.
  const QModelIndex parent = client->watched_id.isEmpty()
                                 ? QModelIndex()
                                 : Resolve(client, client->watched_id);
  client->expecting_children =
      (!client->watched_id.isEmpty() && !parent.isValid())
          ? false
          : model_->canFetchMore(parent) ||
                parent.data(InternetModel::Role_CanLazyLoad).toBool();
  client->loading_since.start();

  client->last_sent = Describe(client);
  emit SendToClient(client_id, client->last_sent);
}

QByteArray InternetBrowser::Describe(Client* client) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::BROWSE);
  cpb::remote::ResponseBrowse* response = msg.mutable_response_browse();
  response->set_node_id(client->watched_id.toStdString());

  QModelIndex parent;
  if (!client->watched_id.isEmpty()) {
    parent = Resolve(client, client->watched_id);
    if (!parent.isValid()) {
      response->set_state(cpb::remote::BROWSE_STATE_GONE);
      client->watching = false;
      const std::string data = msg.SerializeAsString();
      return QByteArray(data.data(), static_cast<int>(data.size()));
    }
  }

  // Loads the children if they haven't been: fetchMore for a grafted library,
  // and rowCount runs a service's LazyPopulate.
  if (model_->canFetchMore(parent)) model_->fetchMore(parent);
  const int rows = model_->rowCount(parent);

  QModelIndexList children;
  bool loading_row = false;
  for (int row = 0; row < rows; ++row) {
    const QModelIndex child = model_->index(row, 0, parent);
    if (child.data(InternetModel::Role_IsDivider).toBool()) continue;
    if (hooks_.item_info(child).library_type ==
        LibraryItem::Type_LoadingIndicator) {
      loading_row = true;
      continue;
    }
    children << child;
  }
  if (!children.isEmpty()) client->expecting_children = false;

  const bool still_loading =
      (loading_row || client->expecting_children) &&
      client->loading_since.elapsed() < loading_timeout_msec_;
  if (children.isEmpty() && parent.isValid() && hooks_.needs_setup(parent)) {
    response->set_state(cpb::remote::BROWSE_STATE_NEEDS_SETUP);
    response->set_message(
        tr("Set up %1 in Clementine's settings on the computer")
            .arg(ServiceName(parent))
            .toStdString());
  } else if (still_loading) {
    response->set_state(cpb::remote::BROWSE_STATE_LOADING);
    // Give up waiting eventually, so a service that never answers doesn't
    // leave the client loading for ever.
    const int remaining =
        loading_timeout_msec_ - int(client->loading_since.elapsed());
    if (!loading_timer_.isActive() ||
        loading_timer_.remainingTime() > remaining) {
      loading_timer_.start(qMax(0, remaining) + 1);
    }
  } else {
    response->set_state(cpb::remote::BROWSE_STATE_READY);
  }

  if (client->nodes.size() > kMaxIdsPerClient) Prune(client);

  // Ids already issued under this node, by where their nodes are now. Ids of
  // nodes that have gone are forgotten.
  QHash<QModelIndex, QString> existing;
  QStringList& siblings = client->children[client->watched_id];
  for (auto it = siblings.begin(); it != siblings.end();) {
    const QModelIndex current = client->nodes.value(*it).index;
    if (current.isValid()) {
      existing[current] = *it;
      ++it;
    } else {
      client->nodes.remove(*it);
      it = siblings.erase(it);
    }
  }

  response->set_offset(client->offset);
  response->set_total_count(children.size());
  const int end = qMin(children.size(), client->offset + client->limit);
  for (int i = client->offset; i < end; ++i) {
    cpb::remote::BrowseNode* node = response->add_nodes();
    node->set_node_id(IdFor(client, client->watched_id, children[i], &existing)
                          .toStdString());
    DescribeNode(children[i], node);
  }

  const std::string data = msg.SerializeAsString();
  return QByteArray(data.data(), static_cast<int>(data.size()));
}

void InternetBrowser::DescribeNode(const QModelIndex& index,
                                   cpb::remote::BrowseNode* node) {
  const ItemInfo info = hooks_.item_info(index);
  const int type = index.data(InternetModel::Role_Type).toInt();
  const bool has_children = model_->hasChildren(index);
  const bool playable = model_->flags(index) & Qt::ItemIsDragEnabled;

  cpb::remote::BrowseNodeKind kind = cpb::remote::BROWSE_NODE_KIND_FOLDER;
  if (type == InternetModel::Type_Service) {
    kind = cpb::remote::BROWSE_NODE_KIND_SERVICE;
  } else if (type == InternetModel::Type_SmartPlaylist ||
             info.library_type == LibraryItem::Type_SmartPlaylist) {
    kind = cpb::remote::BROWSE_NODE_KIND_SMART_PLAYLIST;
  } else if (!has_children && playable) {
    const bool track = info.library_type == LibraryItem::Type_Song ||
                       info.song.length_nanosec() > 0;
    kind = track ? cpb::remote::BROWSE_NODE_KIND_TRACK
                 : cpb::remote::BROWSE_NODE_KIND_STREAM;
  }

  node->set_title(index.data(Qt::DisplayRole).toString().toStdString());
  node->set_kind(kind);
  node->set_children(has_children ? cpb::remote::BROWSE_CHILDREN_SOME
                                  : cpb::remote::BROWSE_CHILDREN_NONE);
  node->set_playability(playable ? cpb::remote::BROWSE_PLAYABILITY_ADDABLE
                                 : cpb::remote::BROWSE_PLAYABILITY_NONE);

  if (info.song.is_valid()) {
    if (!info.song.artist().isEmpty()) {
      node->set_subtitle(info.song.artist().toStdString());
    }
    OutgoingDataCreator::CreateSong(info.song, QImage(), -1,
                                    node->mutable_song());
  }

  if (kind == cpb::remote::BROWSE_NODE_KIND_SERVICE) {
    const QByteArray png = IconPng(index);
    if (!png.isEmpty()) node->set_icon_png(png.constData(), png.size());
  }
}

QString InternetBrowser::IdFor(Client* client, const QString& parent_id,
                               const QModelIndex& index,
                               QHash<QModelIndex, QString>* existing) {
  const QString found = existing->value(index);
  if (!found.isEmpty()) return found;

  const QString id = QString("n%1").arg(client->next_id++);
  client->nodes[id] = Node{QPersistentModelIndex(index), parent_id};
  client->children[parent_id] << id;
  (*existing)[index] = id;
  return id;
}

QModelIndex InternetBrowser::Resolve(Client* client, const QString& id) {
  auto it = client->nodes.find(id);
  if (it == client->nodes.end()) return QModelIndex();
  if (!it->index.isValid()) {
    client->children[it->parent_id].removeAll(id);
    client->nodes.erase(it);
    return QModelIndex();
  }
  return it->index;
}

void InternetBrowser::Prune(Client* client) {
  // Keep the path down to the node being looked at; forget everything else.
  // A client that uses a forgotten id gets GONE and goes back up.
  QHash<QString, Node> kept;
  QString id = client->watched_id;
  while (!id.isEmpty() && client->nodes.contains(id)) {
    kept[id] = client->nodes[id];
    id = client->nodes[id].parent_id;
  }
  client->nodes = kept;
  client->children.clear();
  for (auto it = kept.constBegin(); it != kept.constEnd(); ++it) {
    client->children[it->parent_id] << it.key();
  }
}

QString InternetBrowser::ServiceName(const QModelIndex& index) const {
  QModelIndex top = index;
  while (top.parent().isValid()) top = top.parent();
  return top.data(Qt::DisplayRole).toString();
}

QByteArray InternetBrowser::IconPng(const QModelIndex& index) {
  // Rendering an icon needs a GUI application, which tests don't have.
  if (!qobject_cast<QGuiApplication*>(QCoreApplication::instance())) {
    return QByteArray();
  }
  const QString key = index.data(Qt::DisplayRole).toString();
  auto it = icons_.constFind(key);
  if (it != icons_.constEnd()) return *it;

  QByteArray png;
  const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
  if (!icon.isNull()) {
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    icon.pixmap(kIconSize, kIconSize).save(&buffer, "PNG");
  }
  icons_[key] = png;
  return png;
}

void InternetBrowser::Add(int client_id, Client* client,
                          const cpb::remote::RequestBrowseAdd& request) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::BROWSE_ADD_RESULT);
  cpb::remote::ResponseBrowseAdd* response = msg.mutable_response_browse_add();
  *response->mutable_node_ids() = request.node_ids();

  QModelIndexList indexes;
  bool gone = false;
  for (const std::string& id : request.node_ids()) {
    const QModelIndex index = Resolve(client, QString::fromStdString(id));
    if (!index.isValid()) {
      gone = true;
    } else if (model_->flags(index) & Qt::ItemIsDragEnabled) {
      indexes << index;
    }
  }

  if (gone) {
    response->set_result(cpb::remote::BROWSE_ADD_RESULT_GONE);
  } else if (indexes.isEmpty()) {
    response->set_result(cpb::remote::BROWSE_ADD_RESULT_NOT_PLAYABLE);
  } else {
    const cpb::remote::BrowseAddAction action = request.action();
    QList<QModelIndexList> groups = hooks_.group_by_model(indexes);
    // Each group queued to play next goes in front of the ones queued before
    // it, so queue them last first to keep their order.
    const bool play_next = action == cpb::remote::BROWSE_ADD_ACTION_PLAY_NEXT;
    for (int n = 0; n < groups.size(); ++n) {
      const int i = play_next ? groups.size() - 1 - n : n;
      QMimeData* data = model_->mimeData(groups[i]);
      if (!data) continue;
      if (MimeData* mime = qobject_cast<MimeData*>(data)) {
        const bool first = i == 0;
        mime->override_user_settings_ = true;
        switch (action) {
          case cpb::remote::BROWSE_ADD_ACTION_PLAY_NOW:
            mime->play_now_ = first;
            break;
          case cpb::remote::BROWSE_ADD_ACTION_PLAY_NEXT:
            mime->enqueue_next_now_ = true;
            break;
          case cpb::remote::BROWSE_ADD_ACTION_REPLACE:
            mime->clear_first_ = first;
            mime->play_now_ = first;
            break;
          default:
            break;
        }
      }
      emit AddToPlaylist(data);
    }
    response->set_result(cpb::remote::BROWSE_ADD_RESULT_ADDED);
  }

  Send(client_id, msg);
}

void InternetBrowser::ModelChanged() {
  if (!update_timer_.isActive()) update_timer_.start(update_delay_msec_);
}

void InternetBrowser::SendUpdates() {
  for (auto it = clients_.begin(); it != clients_.end(); ++it) {
    Client* client = &it.value();
    if (!client->watching) continue;
    const QByteArray data = Describe(client);
    if (data == client->last_sent) continue;
    client->last_sent = data;
    emit SendToClient(it.key(), data);
  }
}

void InternetBrowser::Send(int client_id, const cpb::remote::Message& msg) {
  const std::string data = msg.SerializeAsString();
  emit SendToClient(client_id,
                    QByteArray(data.data(), static_cast<int>(data.size())));
}
