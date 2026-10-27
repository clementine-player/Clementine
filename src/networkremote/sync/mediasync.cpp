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

#include "core/logging.h"
#include "core/urlhandler.h"
#include "networkremote/outgoingdatacreator.h"
#include "networkremote/streaming/streamplanner.h"
#include "networkremote/sync/syncsong.h"

const int MediaSync::kMaxItemsPerClient = 20000;

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
  if (msg.type() != cpb::remote::REQUEST_SYNC_MANIFEST) return;

  Client& client = clients_[client_id];
  if (client.token.isEmpty()) {
    client.token = StreamItemTable::NewToken();
    client.peer = NormalisedAddress(QHostAddress(peer_address));
  }

  cpb::remote::Message reply;
  reply.set_type(cpb::remote::SYNC_MANIFEST);
  Manifest(&client, msg.request_sync_manifest(),
           reply.mutable_response_sync_manifest());
  items_->Set(client.token, client.peer, client.items);

  const std::string out = reply.SerializeAsString();
  emit SendToClient(client_id, QByteArray(out.data(), out.size()));
}

void MediaSync::ClientDisconnected(int client_id) {
  auto it = clients_.find(client_id);
  if (it == clients_.end()) return;
  items_->Remove(it->token);
  clients_.erase(it);
}

void MediaSync::Manifest(Client* client,
                         const cpb::remote::RequestSyncManifest& req,
                         cpb::remote::ResponseSyncManifest* response) {
  response->set_source_id(req.source_id());

  if (hooks_.downloads_allowed && !hooks_.downloads_allowed()) {
    response->set_state(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP);
    response->set_message(
        tr("Allow downloads in Clementine's network remote settings to keep "
           "songs on this phone.")
            .toStdString());
    return;
  }

  const SyncSourceId source =
      SyncSourceId::Parse(QString::fromStdString(req.source_id()));
  const SyncResolver::Result result =
      SyncResolver(library_, hooks_.playlists).Resolve(source);
  switch (result.state) {
    case SyncResolver::State::Ready:
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_READY);
      break;
    case SyncResolver::State::Gone:
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_GONE);
      return;
    case SyncResolver::State::Unsupported:
      // Not gone, so the remote keeps the rule for a Clementine that can.
      response->set_state(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP);
      response->set_message(
          tr("This version of Clementine can't sync this yet.").toStdString());
      return;
  }

  RendererCaps caps;
  for (const cpb::remote::AudioFormat& format : req.supported_formats()) {
    RendererCaps::Format f;
    f.mime_type = QString::fromStdString(format.mime_type());
    for (int hz : format.sample_rates_hz()) f.sample_rates_hz << hz;
    f.max_channels = format.max_channels();
    caps.formats << f;
  }
  StreamSettings settings;
  settings.transcode_lossless =
      req.lossless() == cpb::remote::SYNC_LOSSLESS_CONVERT;
  settings.force_encode =
      req.quality() == cpb::remote::SYNC_QUALITY_ALWAYS_CONVERT;

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
    // Only original files for now: converting comes later.
    if (key.isEmpty() ||
        SyncSong::Downloadability(song, handler) !=
            UrlHandler::Downloadability::Downloadable ||
        plan.mode != StreamPlan::Direct) {
      ++skipped;
      continue;
    }

    if (req.limit_bytes() > 0 && bytes + song.filesize() > req.limit_bytes()) {
      break;
    }

    StreamItem item;
    item.plan = plan;
    item.req = playback;
    item.song = song;
    const QString version = SyncSong::ItemVersion(song);
    const int id = Offer(client, key + ':' + version, item);
    if (!id) {
      qLog(Warning) << "A remote has" << kMaxItemsPerClient
                    << "songs on offer already";
      break;
    }
    bytes += song.filesize();

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
    pb->set_size_bytes(song.filesize());
    pb->set_delivery(cpb::remote::SYNC_DELIVERY_ORIGINAL);
  }
  response->set_skipped(skipped);
}

int MediaSync::Offer(Client* client, const QString& key,
                     const StreamItem& item) {
  const auto it = client->ids.constFind(key);
  if (it != client->ids.constEnd()) return *it;
  if (client->items.size() >= kMaxItemsPerClient) return 0;

  StreamItem offered = item;
  offered.id = client->items.size() + 1;
  client->items << offered;
  client->ids[key] = offered.id;
  return offered.id;
}
