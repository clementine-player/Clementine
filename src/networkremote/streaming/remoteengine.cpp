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

#include "remoteengine.h"

#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>
#include <QTimerEvent>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "networkremote/networkremote.h"
#include "networkremote/outgoingdatacreator.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"

namespace {

// Same margins as GstEngine uses for TrackAboutToEnd.
const qint64 kPreloadGapNanosec = 2000 * kNsecPerMsec;
const int kTimerIntervalMsec = 1000;

QByteArray NewToken() {
  QByteArray bytes(16, Qt::Uninitialized);
  QRandomGenerator::system()->fillRange(
      reinterpret_cast<quint32*>(bytes.data()), bytes.size() / 4);
  return bytes.toHex();
}

}  // namespace

RemoteEngine::RemoteEngine(Application* app, const RendererEndpoint& endpoint,
                           const cpb::remote::RendererCapabilities& caps,
                           StreamItemTable* items, QObject* parent)
    : app_(app),
      endpoint_(endpoint),
      items_(items),
      renderer_id_(QString::fromStdString(caps.renderer_id())),
      display_name_(QString::fromStdString(caps.display_name())),
      caps_(RendererCaps::FromProto(caps)),
      token_(NewToken()),
      next_item_id_(1),
      current_sent_(false),
      current_retried_(false),
      state_(Engine::Empty),
      valid_emitted_(false),
      position_ms_(0),
      timer_id_(startTimer(kTimerIntervalMsec)) {
  setParent(parent);
  if (display_name_.isEmpty()) display_name_ = renderer_id_;

  QSettings s;
  s.beginGroup(NetworkRemote::kSettingsGroup);
  settings_.transcode_lossless = s.value("convert_lossless", false).toBool();

  qLog(Info) << "Renderer" << display_name_ << "accepts" << caps_.mime_types()
             << "gapless" << caps_.gapless;
}

RemoteEngine::~RemoteEngine() { items_->Remove(token_); }

void RemoteEngine::PublishItems() {
  QList<StreamItem> items;
  if (current_.id != 0) items << current_;
  if (preloaded_.id != 0) items << preloaded_;
  items_->Set(token_, endpoint_.peer_address, items);
}

Song RemoteEngine::SongForRequest(const MediaPlaybackRequest& req,
                                  bool next) const {
  // The engine interface only carries URLs, so find the song the Player is
  // playing (or about to play) to plan with its metadata.
  PlaylistItemPtr item;
  if (next) {
    Playlist* playlist = app_->playlist_manager()->active();
    const int row = playlist->next_row();
    if (row != -1) item = playlist->item_at(row);
  } else {
    item = app_->player()->GetCurrentItem();
  }

  if (item && item->Url() == req.RequestUrl()) return item->Metadata();

  Song song;
  song.set_url(req.MediaUrl());
  return song;
}

StreamItem RemoteEngine::MakeItem(const MediaPlaybackRequest& req,
                                  const Song& song, bool force_encode) {
  StreamSettings settings = settings_;
  settings.force_encode = force_encode;

  StreamItem item;
  item.id = next_item_id_++;
  item.req = req;
  item.song = song;
  item.plan = StreamPlanner::Plan(req, song, caps_, settings);

  qLog(Debug) << "Item" << item.id << req.MediaUrl() << "->" << item.plan.mode
              << item.plan.mime_type << "(" << item.plan.reason << ")";
  return item;
}

QString RemoteEngine::UrlForItem(const StreamItem& item,
                                 qint64 start_ms) const {
  // The address the renderer reached us on is one it can fetch from.
  QHostAddress host = endpoint_.local_address;
  bool is_v4 = false;
  const quint32 v4 = host.toIPv4Address(&is_v4);
  if (is_v4) host = QHostAddress(v4);

  QUrl url;
  url.setScheme("http");
  url.setHost(host.toString());
  url.setPort(endpoint_.local_port);
  url.setPath(
      QString("/s/%1/%2").arg(QString::fromLatin1(token_)).arg(item.id));
  if (start_ms > 0) url.setQuery(QString("t=%1").arg(start_ms));
  return url.toString();
}

void RemoteEngine::FillRenderItem(const StreamItem& item,
                                  cpb::remote::RenderItem* pb) const {
  pb->set_item_id(item.id);
  pb->set_url(UrlForItem(item).toStdString());
  pb->set_mime_type(item.plan.mime_type.toStdString());
  if (item.plan.length_nanosec > 0) {
    pb->set_length_ms(item.plan.length_nanosec / kNsecPerMsec);
  }

  if (item.plan.mode == StreamPlan::Direct) {
    pb->set_mode(cpb::remote::STREAM_MODE_DIRECT);
    pb->set_seek_method(cpb::remote::SEEK_METHOD_BYTE_RANGE);
  } else {
    pb->set_mode(cpb::remote::STREAM_MODE_PIPELINE);
    pb->set_seek_method(item.plan.length_nanosec > 0
                            ? cpb::remote::SEEK_METHOD_NEW_URL
                            : cpb::remote::SEEK_METHOD_NONE);
  }

  OutgoingDataCreator::CreateSong(item.song, QImage(), -1, pb->mutable_song());
}

void RemoteEngine::Send(cpb::remote::MsgType type, cpb::remote::Message* msg) {
  cpb::remote::Message empty;
  if (!msg) msg = &empty;
  msg->set_type(type);
  const std::string data = msg->SerializeAsString();
  emit SendToClient(endpoint_.client_id,
                    QByteArray(data.data(), static_cast<int>(data.size())));
}

void RemoteEngine::SendLoad(const StreamItem& item, qint64 start_ms,
                            bool playing) {
  cpb::remote::Message msg;
  cpb::remote::RequestRenderLoad* load = msg.mutable_request_render_load();
  FillRenderItem(item, load->mutable_item());
  if (item.plan.mode == StreamPlan::Pipeline && start_ms > 0) {
    // Pipeline output can't be seeked into, so hand over a URL that starts
    // there.
    load->mutable_item()->set_url(UrlForItem(item, start_ms).toStdString());
  }
  load->set_start_ms(start_ms);
  load->set_start_state(playing ? cpb::remote::LOAD_START_STATE_PLAYING
                                : cpb::remote::LOAD_START_STATE_PAUSED);
  Send(cpb::remote::RENDER_LOAD, &msg);

  position_ms_ = start_ms;
  position_clock_.start();
}

void RemoteEngine::SetState(Engine::State state) {
  if (state == state_) return;
  state_ = state;
  emit StateChanged(state);
}

void RemoteEngine::FailItem(const QString& message) {
  const MediaPlaybackRequest req = current_.req;
  current_ = StreamItem();
  current_sent_ = false;
  PublishItems();
  SetState(Engine::Error);
  emit Error(message);
  emit InvalidMediaRequested(req);
}

void RemoteEngine::StartPreloading(const MediaPlaybackRequest& req, bool,
                                   qint64, qint64) {
  // Without gapless support the renderer can't queue anything; the next
  // track is loaded when this one ends.
  if (!caps_.gapless || current_.id == 0) return;

  preloaded_ = MakeItem(req, SongForRequest(req, true), false);
  if (preloaded_.plan.mode == StreamPlan::Unplayable) {
    preloaded_ = StreamItem();
    return;
  }
  PublishItems();

  cpb::remote::Message msg;
  FillRenderItem(preloaded_,
                 msg.mutable_request_render_preload()->mutable_item());
  Send(cpb::remote::RENDER_PRELOAD, &msg);
}

bool RemoteEngine::Load(const MediaPlaybackRequest& req,
                        Engine::TrackChangeFlags change, bool force_stop_at_end,
                        quint64 beginning_nanosec, qint64 end_nanosec) {
  Engine::Base::Load(req, change, force_stop_at_end, beginning_nanosec,
                     end_nanosec);

  // After a gapless transition the renderer is already playing this.
  if (change & Engine::Auto && current_sent_ && current_.id != 0 &&
      current_.req.MediaUrl() == req.MediaUrl()) {
    return true;
  }

  preloaded_ = StreamItem();
  current_ = MakeItem(req, SongForRequest(req, false), false);
  current_sent_ = false;
  current_retried_ = false;
  valid_emitted_ = false;
  position_ms_ = 0;
  PublishItems();

  if (current_.plan.mode == StreamPlan::Unplayable) {
    const QString message = tr("%1 can't play this track: %2")
                                .arg(display_name_, current_.plan.reason);
    // Let the caller finish before the Player skips to the next track.
    QTimer::singleShot(0, this, [this, message]() { FailItem(message); });
    return false;
  }
  return true;
}

bool RemoteEngine::Play(quint64 offset_nanosec) {
  if (current_.id == 0) return false;

  if (!current_sent_) {
    SendLoad(current_, offset_nanosec / kNsecPerMsec, true);
    current_sent_ = true;
    SetState(Engine::Playing);
    return true;
  }

  if (offset_nanosec != 0) Seek(offset_nanosec);
  if (state_ != Engine::Playing) Unpause();
  return true;
}

void RemoteEngine::Stop(bool) {
  Send(cpb::remote::RENDER_STOP);
  current_ = StreamItem();
  preloaded_ = StreamItem();
  current_sent_ = false;
  position_ms_ = 0;
  playback_req_ = MediaPlaybackRequest();
  PublishItems();
  SetState(Engine::Empty);
}

void RemoteEngine::Pause() {
  if (state_ != Engine::Playing) return;
  Send(cpb::remote::RENDER_PAUSE);
  position_ms_ = position_nanosec() / kNsecPerMsec;
  position_clock_.start();
  SetState(Engine::Paused);
}

void RemoteEngine::Unpause() {
  if (state_ != Engine::Paused) return;
  Send(cpb::remote::RENDER_PLAY);
  position_clock_.start();
  SetState(Engine::Playing);
}

void RemoteEngine::Seek(quint64 offset_nanosec) {
  if (current_.id == 0) return;
  const qint64 position_ms = offset_nanosec / kNsecPerMsec;

  cpb::remote::Message msg;
  cpb::remote::RequestRenderSeek* seek = msg.mutable_request_render_seek();
  seek->set_item_id(current_.id);
  seek->set_position_ms(position_ms);
  if (current_.plan.mode == StreamPlan::Pipeline) {
    seek->set_url(UrlForItem(current_, position_ms).toStdString());
  }
  Send(cpb::remote::RENDER_SEEK, &msg);

  position_ms_ = position_ms;
  position_clock_.start();
}

qint64 RemoteEngine::position_nanosec() const {
  qint64 ms = position_ms_;
  if (state_ == Engine::Playing && position_clock_.isValid()) {
    ms += position_clock_.elapsed();
  }
  const qint64 length = length_nanosec();
  const qint64 nanosec = ms * kNsecPerMsec;
  return length > 0 ? qMin(nanosec, length) : nanosec;
}

qint64 RemoteEngine::length_nanosec() const {
  return current_.id == 0 ? 0 : current_.plan.length_nanosec;
}

void RemoteEngine::SetVolumeSW(uint) {
  // Renderers apply their own curve; send the slider value.
  cpb::remote::Message msg;
  msg.mutable_request_render_volume()->set_volume(volume_);
  Send(cpb::remote::RENDER_SET_VOLUME, &msg);
}

void RemoteEngine::timerEvent(QTimerEvent* e) {
  if (e->timerId() != timer_id_) return;
  if (state_ != Engine::Playing) return;

  const qint64 length = length_nanosec();
  if (length <= 0) return;

  const qint64 fudge = (kTimerIntervalMsec + 100) * kNsecPerMsec;
  if (length - position_nanosec() < kPreloadGapNanosec + fudge) {
    EmitAboutToEnd();
  }
}

void RemoteEngine::HandleMessage(const cpb::remote::Message& msg) {
  switch (msg.type()) {
    case cpb::remote::RENDERER_STATUS:
      HandleStatus(msg.renderer_status());
      break;
    case cpb::remote::RENDERER_TRACK_ENDED:
      HandleTrackEnded(msg.renderer_track_ended().item_id());
      break;
    case cpb::remote::RENDERER_ERROR:
      HandleError(msg.renderer_error());
      break;
    default:
      break;
  }
}

void RemoteEngine::HandleStatus(const cpb::remote::RendererStatus& status) {
  // Reports about an older item can arrive after a track change.
  if (current_.id == 0 || status.item_id() != current_.id) return;

  position_ms_ = status.position_ms();
  position_clock_.start();

  switch (status.state()) {
    case cpb::remote::RENDERER_STATE_LOADING:
    case cpb::remote::RENDERER_STATE_BUFFERING:
    case cpb::remote::RENDERER_STATE_PLAYING:
      SetState(Engine::Playing);
      if (status.state() == cpb::remote::RENDERER_STATE_PLAYING &&
          !valid_emitted_) {
        valid_emitted_ = true;
        emit ValidMediaRequested(current_.req);
      }
      break;
    case cpb::remote::RENDERER_STATE_PAUSED:
      SetState(Engine::Paused);
      break;
    case cpb::remote::RENDERER_STATE_IDLE:
      SetState(Engine::Idle);
      break;
    default:
      break;
  }
}

void RemoteEngine::HandleTrackEnded(int item_id) {
  if (current_.id == 0 || item_id != current_.id) return;

  if (preloaded_.id != 0) {
    // The renderer has moved on to the preloaded item without a gap.
    current_ = preloaded_;
    preloaded_ = StreamItem();
    current_sent_ = true;
    current_retried_ = false;
    valid_emitted_ = false;
    position_ms_ = 0;
    position_clock_.start();
    PublishItems();
  } else {
    current_sent_ = false;
    SetState(Engine::Idle);
  }
  emit TrackEnded();
}

void RemoteEngine::HandleError(const cpb::remote::RendererError& error) {
  const QString message = QString::fromStdString(error.message());
  qLog(Warning) << "Renderer" << display_name_ << "error on item"
                << error.item_id() << ":" << message;

  switch (error.scope()) {
    case cpb::remote::RENDERER_ERROR_SCOPE_RENDERER:
      emit RendererFailed();
      return;

    case cpb::remote::RENDERER_ERROR_SCOPE_TRANSIENT:
      if (current_.id != 0 && error.item_id() == current_.id) {
        SendLoad(current_, position_nanosec() / kNsecPerMsec,
                 state_ == Engine::Playing);
      }
      return;

    default:
      break;
  }

  if (current_.id == 0 || error.item_id() != current_.id) return;

  // The renderer said it could play the original but couldn't: try once
  // more with the pipeline encoding to something it certainly asked for.
  if (!current_retried_) {
    // If it never started, retry from where it was loaded; the interpolated
    // position has been counting since the load.
    const qint64 position_ms =
        valid_emitted_ ? position_nanosec() / kNsecPerMsec : position_ms_;
    StreamItem retry = MakeItem(current_.req, current_.song, true);
    if (retry.plan.mode != StreamPlan::Unplayable) {
      current_ = retry;
      current_retried_ = true;
      PublishItems();
      SendLoad(current_, position_ms, state_ != Engine::Paused);
      return;
    }
  }

  FailItem(message.isEmpty()
               ? tr("%1 couldn't play this track").arg(display_name_)
               : message);
}
