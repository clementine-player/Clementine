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

#include "upnpengine.h"

#include <QSettings>
#include <QTimer>
#include <QTimerEvent>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "networkremote/networkremote.h"
#include "networkremote/streaming/rendererregistry.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"

namespace {

// Same margins as ChromecastEngine.
const qint64 kPreloadGapNanosec = 2000 * kNsecPerMsec;
const int kTimerIntervalMsec = 1000;

}  // namespace

UpnpEngine::UpnpEngine(Application* app, const UpnpDevice& device,
                       const RendererRegistry* registry, StreamItemTable* items,
                       QObject* parent)
    : app_(app),
      device_(device),
      registry_(registry),
      items_(items),
      token_(StreamItemTable::NewToken()),
      next_item_id_(1),
      in_control_(false),
      current_sent_(false),
      current_retried_(false),
      valid_emitted_(false),
      position_base_ms_(0),
      state_(Engine::Empty),
      timer_id_(startTimer(kTimerIntervalMsec)) {
  setParent(parent);
  player_.SetDevice(device_);

  QSettings s;
  s.beginGroup(NetworkRemote::kSettingsGroup);
  settings_.transcode_lossless = s.value("convert_lossless", false).toBool();

  connect(&player_, &UpnpMediaPlayer::StateChanged, this,
          &UpnpEngine::PlayerStateChanged);
  connect(&player_, &UpnpMediaPlayer::TrackEnded, this,
          &UpnpEngine::PlayerTrackEnded);
  connect(&player_, &UpnpMediaPlayer::Error, this, &UpnpEngine::PlayerError);
  connect(&player_, &UpnpMediaPlayer::Lost, this, &UpnpEngine::PlayerLost);
}

UpnpEngine::~UpnpEngine() { items_->Remove(token_); }

void UpnpEngine::SetDevice(const UpnpDevice& device) {
  const bool moved = device.address != device_.address;
  device_ = device;
  player_.SetDevice(device_);
  // The token is only for the old address.
  if (moved) PublishItems();
}

void UpnpEngine::PublishItems() {
  QList<StreamItem> items;
  if (current_.id != 0) items << current_;
  items_->Set(token_, device_.address, items);
}

Song UpnpEngine::SongForRequest(const MediaPlaybackRequest& req) const {
  // As RemoteEngine does: the engine interface only carries URLs.
  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (item && item->Url() == req.RequestUrl()) return item->Metadata();

  Song song;
  song.set_url(req.MediaUrl());
  return song;
}

StreamItem UpnpEngine::MakeItem(const MediaPlaybackRequest& req,
                                const Song& song, bool force_encode) {
  StreamSettings settings = settings_;
  settings.force_encode = force_encode;

  StreamItem item;
  item.id = next_item_id_++;
  item.req = req;
  item.song = song;
  item.plan = StreamPlanner::Plan(req, song, device_.caps, settings);
  item.content_type = device_.DeviceMimeType(item.plan.mime_type);

  qLog(Debug) << "UPnP item" << item.id << req.MediaUrl() << "->"
              << item.plan.mode << item.plan.mime_type << "("
              << item.plan.reason << ")";
  return item;
}

QUrl UpnpEngine::UrlForItem(const StreamItem& item, qint64 start_ms) const {
  QUrl url = registry_->MediaBaseUrl(device_.address);
  if (url.isEmpty()) return url;
  url.setPath(
      QString("/s/%1/%2").arg(QString::fromLatin1(token_)).arg(item.id));
  if (start_ms > 0) url.setQuery(QString("t=%1").arg(start_ms));
  return url;
}

void UpnpEngine::SendLoad(qint64 start_ms, bool playing) {
  if (current_.id == 0) return;

  // Pipeline output can't be seeked into, so it gets a URL that starts at
  // the position instead.
  const bool pipeline = current_.plan.mode == StreamPlan::Pipeline;
  const QUrl url = UrlForItem(current_, pipeline ? start_ms : 0);
  if (url.isEmpty()) {
    // Every track would fail the same way, so rather than skip through the
    // playlist, give up on the renderer. Not from inside Play(), whose
    // caller would be told to play the next track before it's finished.
    const QString message = registry_->ReachMessage(
        registry_->ReachFor(device_.address), device_.name);
    qLog(Warning) << message;
    current_sent_ = false;
    QTimer::singleShot(0, this, [this, message]() {
      emit Error(message);
      emit DeviceFailed();
    });
    return;
  }

  UpnpMediaInfo info;
  info.url = url;
  info.content_type = current_.ContentType();
  info.title = current_.song.PrettyTitle();
  info.artist = current_.song.artist();
  info.album = current_.song.album();
  if (current_.plan.length_nanosec > 0) {
    info.duration_ms = current_.plan.length_nanosec / kNsecPerMsec;
  }
  position_base_ms_ = pipeline ? start_ms : 0;
  qLog(Info) << "Playing" << url << "on" << device_;
  player_.Load(info, pipeline ? 0 : start_ms, playing);
  current_sent_ = true;
  in_control_ = true;
}

void UpnpEngine::SetState(Engine::State state) {
  if (state == state_) return;
  state_ = state;
  emit StateChanged(state);
}

void UpnpEngine::FailItem(const QString& message) {
  const MediaPlaybackRequest req = current_.req;
  current_ = StreamItem();
  current_sent_ = false;
  PublishItems();
  SetState(Engine::Error);
  emit Error(message);
  emit InvalidMediaRequested(req);
}

bool UpnpEngine::Load(const MediaPlaybackRequest& req,
                      Engine::TrackChangeFlags change, bool force_stop_at_end,
                      quint64 beginning_nanosec, qint64 end_nanosec) {
  Engine::Base::Load(req, change, force_stop_at_end, beginning_nanosec,
                     end_nanosec);

  current_ = MakeItem(req, SongForRequest(req), false);
  current_sent_ = false;
  current_retried_ = false;
  valid_emitted_ = false;
  position_base_ms_ = 0;
  PublishItems();

  if (current_.plan.mode == StreamPlan::Unplayable) {
    const QString message = tr("%1 can't play this track: %2")
                                .arg(device_.name, current_.plan.reason);
    // Let the caller finish before the Player skips to the next track.
    QTimer::singleShot(0, this, [this, message]() { FailItem(message); });
    return false;
  }
  return true;
}

bool UpnpEngine::Play(quint64 offset_nanosec) {
  if (current_.id == 0) return false;

  if (!current_sent_) {
    SendLoad(offset_nanosec / kNsecPerMsec, true);
    SetState(Engine::Playing);
    return true;
  }

  if (offset_nanosec != 0) Seek(offset_nanosec);
  if (state_ != Engine::Playing) Unpause();
  return true;
}

void UpnpEngine::Stop(bool) {
  if (current_sent_) player_.Stop();
  current_ = StreamItem();
  current_sent_ = false;
  // Including when the router moves playback elsewhere.
  in_control_ = false;
  position_base_ms_ = 0;
  playback_req_ = MediaPlaybackRequest();
  PublishItems();
  SetState(Engine::Empty);
}

void UpnpEngine::Pause() {
  if (state_ != Engine::Playing) return;
  player_.Pause();
  SetState(Engine::Paused);
}

void UpnpEngine::Unpause() {
  if (state_ != Engine::Paused) return;
  player_.Play();
  SetState(Engine::Playing);
}

void UpnpEngine::Seek(quint64 offset_nanosec) {
  if (current_.id == 0 || !current_sent_) return;
  const qint64 position_ms = offset_nanosec / kNsecPerMsec;

  if (current_.plan.mode == StreamPlan::Direct) {
    player_.Seek(position_ms);
  } else if (current_.plan.length_nanosec > 0) {
    // A new URL that starts there.
    SendLoad(position_ms, state_ != Engine::Paused);
  }
}

qint64 UpnpEngine::position_nanosec() const {
  const qint64 nanosec =
      (position_base_ms_ + player_.position_ms()) * kNsecPerMsec;
  const qint64 length = length_nanosec();
  return length > 0 ? qMin(nanosec, length) : nanosec;
}

qint64 UpnpEngine::length_nanosec() const {
  if (current_.id == 0) return 0;
  if (current_.plan.length_nanosec > 0) return current_.plan.length_nanosec;
  return player_.duration_ms() * kNsecPerMsec;
}

QString UpnpEngine::output_icon() const {
  if (device_.group) return "speaker-group";
  if (device_.video) return "tv";
  return "speaker";
}

void UpnpEngine::SetVolumeSW(uint) {
  // Not before playing on it; see the class comment.
  if (!in_control_) return;
  player_.SetVolume(volume_);
}

void UpnpEngine::PlayerStateChanged(UpnpMediaPlayer::State state) {
  if (!current_sent_) return;
  switch (state) {
    case UpnpMediaPlayer::Buffering:
      SetState(Engine::Playing);
      break;
    case UpnpMediaPlayer::Playing:
      SetState(Engine::Playing);
      if (!valid_emitted_) {
        valid_emitted_ = true;
        emit ValidMediaRequested(current_.req);
      }
      break;
    case UpnpMediaPlayer::Paused:
      SetState(Engine::Paused);
      break;
    case UpnpMediaPlayer::Idle:
      // TrackEnded, Error or Lost says why.
      break;
  }
}

void UpnpEngine::PlayerTrackEnded() {
  if (current_.id == 0) return;
  current_sent_ = false;
  SetState(Engine::Idle);
  emit TrackEnded();
}

void UpnpEngine::PlayerError(const QString& message) {
  qLog(Warning) << device_.name << "error:" << message;
  if (current_.id == 0 || !current_sent_) return;

  // The renderer was sent the original but couldn't play it: try once more,
  // converted to something it certainly plays.
  if (!current_retried_) {
    const qint64 position_ms = position_nanosec() / kNsecPerMsec;
    StreamItem retry = MakeItem(current_.req, current_.song, true);
    if (retry.plan.mode != StreamPlan::Unplayable) {
      current_ = retry;
      current_retried_ = true;
      PublishItems();
      SendLoad(position_ms, state_ != Engine::Paused);
      return;
    }
  }
  FailItem(message);
}

void UpnpEngine::PlayerLost(const QString& reason) {
  qLog(Warning) << "Lost" << device_.name << ":" << reason;
  current_sent_ = false;
  in_control_ = false;
  if (state_ == Engine::Playing || state_ == Engine::Paused) {
    emit DeviceFailed();
  }
}

void UpnpEngine::timerEvent(QTimerEvent* e) {
  if (e->timerId() != timer_id_) return;
  if (state_ != Engine::Playing) return;

  const qint64 length = length_nanosec();
  if (length <= 0) return;

  const qint64 fudge = (kTimerIntervalMsec + 100) * kNsecPerMsec;
  if (length - position_nanosec() < kPreloadGapNanosec + fudge) {
    EmitAboutToEnd();
  }
}
