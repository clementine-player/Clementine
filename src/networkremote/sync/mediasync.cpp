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

#include "mediasync.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <limits>

#include "core/logging.h"
#include "core/urlhandler.h"
#include "networkremote/outgoingdatacreator.h"
#include "networkremote/streaming/mediahttpserver.h"
#include "networkremote/streaming/streamplanner.h"
#include "networkremote/sync/syncsong.h"

const int MediaSync::kMaxItemsPerClient = 20000;

namespace {

const int kMaxCoverKeyCacheEntries = 10000;

// The file holding |song|'s cover, as Clementine shows it, or empty if it has
// none or it's somewhere else. A cover in the song's own file isn't sent: the
// remote reads it from the song.
QString CoverFile(const Song& song) {
  QString art = song.art_manual();
  if (art == Song::kManuallyUnsetCover || art == Song::kEmbeddedCover) {
    return QString();
  }
  if (art.isEmpty()) art = song.art_automatic();
  if (art.isEmpty() || art == Song::kEmbeddedCover ||
      art == Song::kManuallyUnsetCover || art.contains("://")) {
    return QString();
  }
  return art;
}

}  // namespace

MediaSync::MediaSync(LibraryBackend* library, const Hooks& hooks,
                     std::shared_ptr<StreamItemTable> items, QObject* parent)
    : QObject(parent), library_(library), hooks_(hooks), items_(items) {}

MediaSync::~MediaSync() {
  for (const Client& client : clients_) items_->Remove(client.token);
}

void MediaSync::HandleMessage(int client_id, const QString& peer_address,
                              const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;
  if (msg.type() == cpb::remote::REQUEST_SYNC_SOURCES) {
    cpb::remote::Message reply;
    reply.set_type(cpb::remote::SYNC_SOURCES);
    cpb::remote::ResponseSyncSources* response =
        reply.mutable_response_sync_sources();
    if (hooks_.sources) {
      for (const cpb::remote::SyncSource& source : hooks_.sources()) {
        *response->add_sources() = source;
      }
    }
    const std::string out = reply.SerializeAsString();
    emit SendToClient(client_id, QByteArray(out.data(), out.size()));
    return;
  }
  if (msg.type() != cpb::remote::REQUEST_SYNC_MANIFEST) return;

  Client* client = &clients_[client_id];
  if (client->token.isEmpty()) {
    client->token = StreamItemTable::NewToken();
    client->peer = NormalisedAddress(QHostAddress(peer_address));
  }

  cpb::remote::Message reply;
  reply.set_type(cpb::remote::SYNC_MANIFEST);
  Manifest(client, msg.request_sync_manifest(),
           reply.mutable_response_sync_manifest());
  // Remove empty entries so disallowed or cleared syncs drop HTTP tokens.
  if (client->items.isEmpty()) {
    items_->Remove(client->token);
  } else {
    items_->Set(client->token, client->peer, client->items.values());
  }

  const std::string out = reply.SerializeAsString();
  emit SendToClient(client_id, QByteArray(out.data(), out.size()));
}

void MediaSync::ClientDisconnected(int client_id) {
  auto it = clients_.find(client_id);
  if (it == clients_.end()) return;
  items_->Remove(it->token);
  clients_.erase(it);
}

void MediaSync::ClearClients() {
  for (const Client& client : clients_) items_->Remove(client.token);
  clients_.clear();
}

void MediaSync::Manifest(Client* client,
                         const cpb::remote::RequestSyncManifest& req,
                         cpb::remote::ResponseSyncManifest* response) {
  const QString source_key = QString::fromStdString(req.source_id());
  response->set_source_id(req.source_id());

  if (hooks_.downloads_allowed && !hooks_.downloads_allowed()) {
    // Revoke existing /d/ tokens immediately when downloads are disallowed.
    ClearClientOffers(client);
    response->set_state(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP);
    response->set_message(
        tr("Allow downloads in Clementine's network remote settings to keep "
           "songs on this phone.")
            .toStdString());
    return;
  }

  const SyncSourceId source = SyncSourceId::Parse(source_key);
  const SyncResolver::Result result =
      SyncResolver(library_, hooks_.playlists, hooks_.smart_playlists)
          .Resolve(source, req.has_seed(), req.seed());
  switch (result.state) {
    case SyncResolver::State::Ready:
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_READY);
      break;
    case SyncResolver::State::Gone:
      ReleaseSource(client, source_key);
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_GONE);
      return;
    case SyncResolver::State::Unsupported:
      // Not gone, so the remote keeps the rule for a Clementine that can.
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP);
      response->set_message(
          tr("This version of Clementine can't sync this yet.").toStdString());
      return;
  }

  const RendererCaps caps = RendererCaps::FromSyncManifest(req);
  // Desktop sync currently only serves original files (SYNC_DELIVERY_ORIGINAL)
  // and leaves ResponseClementineInfo.encodable_formats unset; keep transcode
  // flags disabled so convert requests serve playable originals instead of
  // dropping 100% of tracks.
  StreamSettings settings;

  QSet<int> old_ids = client->source_items.value(source_key);
  QSet<int> new_ids;
  QSet<int> added_ids;

  int skipped = 0;
  qint64 bytes = 0;
  for (const Song& song : result.songs) {
    if (req.limit_items() > 0 && response->items_size() >= req.limit_items()) {
      break;
    }

    const UrlHandler* handler =
        hooks_.url_handler ? hooks_.url_handler(song) : nullptr;
    const QString key = SyncSong::ItemKey(song);
    const MediaPlaybackRequest playback(song.url());
    const StreamPlan plan = StreamPlanner::Plan(playback, song, caps, settings);
    if (key.isEmpty() ||
        SyncSong::Downloadability(song, handler) !=
            UrlHandler::Downloadability::Downloadable ||
        plan.mode != StreamPlan::Direct) {
      ++skipped;
      continue;
    }

    // Clamp unknown (-1) sizes to 0 and subtract to avoid signed overflow.
    const qint64 song_size = qMax<qint64>(0, song.filesize());
    if (req.limit_bytes() > 0 &&
        (bytes > req.limit_bytes() || song_size > req.limit_bytes() - bytes)) {
      break;
    }

    const QString cover_file = CoverFile(song);
    const QString cover_key =
        cover_file.isEmpty() ? QString() : CoverKeyOf(cover_file);

    StreamItem item;
    item.plan = plan;
    item.req = playback;
    item.song = song;
    if (!cover_key.isEmpty()) item.cover_file = cover_file;
    const QString version = SyncSong::ItemVersion(song);
    item.etag = '"' + (key + ':' + version).toUtf8() + '"';
    const int id = Offer(client, source_key, key + ':' + version, item,
                         &old_ids, &new_ids, &added_ids);
    if (!id) {
      qLog(Warning) << "A remote has" << kMaxItemsPerClient
                    << "songs on offer already";
      // Roll back this source's offer and return NEEDS_SETUP (never READY)
      // so clients don't treat a truncated manifest as complete and delete
      // already-synced songs.
      for (int added_id : added_ids) {
        ReleaseItem(client, added_id);
      }
      ReleaseSource(client, source_key);
      response->clear_items();
      response->clear_skipped();
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP);
      response->set_message(
          tr("This sync exceeds Clementine's %1-song offer limit per remote.")
              .arg(kMaxItemsPerClient)
              .toStdString());
      return;
    }
    bytes += song_size;

    cpb::remote::SyncItem* pb = response->add_items();
    pb->set_item_key(key.toStdString());
    pb->set_version(version.toStdString());
    OutgoingDataCreator::CreateSong(song, QImage(), -1, pb->mutable_song());
    pb->mutable_song()->set_downloadable(
        cpb::remote::DOWNLOADABILITY_DOWNLOADABLE);
    pb->set_url(QString("/d/%1/%2")
                    .arg(QString::fromLatin1(client->token))
                    .arg(id)
                    .toStdString());
    pb->set_mime_type(plan.mime_type.toStdString());
    pb->set_size_bytes(song_size);
    pb->set_delivery(cpb::remote::SYNC_DELIVERY_ORIGINAL);
    if (!cover_key.isEmpty()) {
      cpb::remote::SyncArt* art = pb->mutable_art();
      art->set_key(cover_key.toStdString());
      art->set_url(QString("/a/%1/%2")
                       .arg(QString::fromLatin1(client->token))
                       .arg(id)
                       .toStdString());
    }
  }

  // Reconcile source membership and release dropped tracks so re-syncing
  // changing playlists doesn't leak slots toward kMaxItemsPerClient.
  if (new_ids.isEmpty()) {
    client->source_items.remove(source_key);
  } else {
    client->source_items[source_key] = new_ids;
  }
  for (int old_id : old_ids) {
    if (!new_ids.contains(old_id)) {
      ReleaseItem(client, old_id);
    }
  }
  response->set_skipped(skipped);
}

int MediaSync::Offer(Client* client, const QString& source_id,
                     const QString& key, const StreamItem& item,
                     QSet<int>* old_ids, QSet<int>* new_ids,
                     QSet<int>* added_ids) {
  const auto it = client->ids.constFind(key);
  if (it != client->ids.constEnd()) {
    const int id = *it;
    // The song's the same, but its cover may not be.
    client->items[id].cover_file = item.cover_file;
    // Deduplicate across sources with ref_counts so shared tracks stay
    // live until every referencing source drops them.
    if (!new_ids->contains(id)) {
      new_ids->insert(id);
      if (!old_ids->contains(id)) {
        client->ref_counts[id] += 1;
        added_ids->insert(id);
      }
    }
    return id;
  }

  if (client->items.size() >= kMaxItemsPerClient) {
    // Evict a stale item exclusive to old_ids before failing on capacity.
    int evict_id = 0;
    for (int candidate : *old_ids) {
      if (!new_ids->contains(candidate) &&
          client->ref_counts.value(candidate) <= 1) {
        evict_id = candidate;
        break;
      }
    }
    if (evict_id == 0) return 0;
    old_ids->remove(evict_id);
    if (client->source_items.contains(source_id)) {
      client->source_items[source_id].remove(evict_id);
    }
    client->ref_counts.remove(evict_id);
    const QString evict_key = client->keys.take(evict_id);
    if (!evict_key.isEmpty()) client->ids.remove(evict_key);
    client->items.remove(evict_id);
  }

  // Allocate non-colliding IDs instead of items.size() + 1 because
  // evictions leave gaps in the map.
  int id = client->next_id;
  while (id <= 0 || client->items.contains(id)) {
    id = (id == std::numeric_limits<int>::max()) ? 1 : id + 1;
  }
  client->next_id = (id == std::numeric_limits<int>::max()) ? 1 : id + 1;

  StreamItem offered = item;
  offered.id = id;
  client->items.insert(id, offered);
  client->ids.insert(key, id);
  client->keys.insert(id, key);
  client->ref_counts.insert(id, 1);
  new_ids->insert(id);
  added_ids->insert(id);
  return id;
}

void MediaSync::ReleaseItem(Client* client, int id) {
  auto rc = client->ref_counts.find(id);
  if (rc == client->ref_counts.end()) return;
  if (--(*rc) > 0) return;
  client->ref_counts.erase(rc);
  const QString key = client->keys.take(id);
  if (!key.isEmpty()) client->ids.remove(key);
  client->items.remove(id);
}

void MediaSync::ReleaseSource(Client* client, const QString& source_id) {
  auto it = client->source_items.find(source_id);
  if (it == client->source_items.end()) return;
  const QSet<int> removed = it.value();
  client->source_items.erase(it);
  for (int id : removed) {
    ReleaseItem(client, id);
  }
}

void MediaSync::ClearClientOffers(Client* client) {
  client->items.clear();
  client->ids.clear();
  client->keys.clear();
  client->source_items.clear();
  client->ref_counts.clear();
}

QString MediaSync::CoverKeyOf(const QString& path) {
  const QFileInfo info(path);
  // Validate size and image MIME type before hashing on the main thread
  // to match MediaHttpServer::WriteCover limits.
  if (!info.isFile() || info.size() <= 0 ||
      info.size() > MediaHttpServer::kMaxCoverFileSize) {
    return QString();
  }
  const QString mime = QMimeDatabase().mimeTypeForFile(path).name();
  if (!mime.startsWith("image/")) {
    return QString();
  }

  auto it = cover_keys_.find(path);
  if (it != cover_keys_.end() && it->modified == info.lastModified() &&
      it->size == info.size()) {
    return it->key;
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return QString();
  QCryptographicHash hash(QCryptographicHash::Sha256);
  if (!hash.addData(&file)) return QString();

  if (cover_keys_.size() >= kMaxCoverKeyCacheEntries) {
    cover_keys_.clear();
  }

  CoverKey cover;
  cover.modified = info.lastModified();
  cover.size = info.size();
  cover.key = QString::fromLatin1(hash.result().toHex());
  cover_keys_[path] = cover;
  return cover.key;
}
