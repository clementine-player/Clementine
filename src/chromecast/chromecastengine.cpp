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

#include "chromecastengine.h"

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

// Same margins as RemoteEngine.
const qint64 kPreloadGapNanosec = 2000 * kNsecPerMsec;
const int kTimerIntervalMsec = 1000;

RendererCaps::Format MakeFormat(const QString& mime_type,
                                const QList<int>& sample_rates = {}) {
  RendererCaps::Format format;
  format.mime_type = mime_type;
  format.sample_rates_hz = sample_rates;
  return format;
}

}  // namespace

RendererCaps ChromecastEngine::Caps() {
  // From Google's list of the media every Cast device supports. Ogg isn't
  // there, so Vorbis and Opus are converted, to MP3.
  RendererCaps caps;
  caps.formats << MakeFormat("audio/mpeg") << MakeFormat("audio/mp4")
               << MakeFormat("audio/aac")
               << MakeFormat("audio/flac", {8000, 11025, 16000, 22050, 32000,
                                            44100, 48000, 88200, 96000})
               << MakeFormat("audio/wav");
  caps.http_range = true;
  return caps;
}

ChromecastEngine::ChromecastEngine(Application* app, const CastDevice& device,
                                   const RendererRegistry* registry,
                                   StreamItemTable* items, QObject* parent)
    : app_(app),
      device_(device),
      registry_(registry),
      items_(items),
      token_(StreamItemTable::NewToken()),
      next_item_id_(1),
      player_(&channel_),
      current_sent_(false),
      current_retried_(false),
      valid_emitted_(false),
      position_base_ms_(0),
      state_(Engine::Empty),
      timer_id_(startTimer(kTimerIntervalMsec)) {
  setParent(parent);

  QSettings s;
  s.beginGroup(NetworkRemote::kSettingsGroup);
  settings_.transcode_lossless = s.value("convert_lossless", false).toBool();

  connect(&channel_, SIGNAL(Opened()), SLOT(ChannelOpened()));
  connect(&channel_, SIGNAL(Closed(QString)), SLOT(ChannelClosed(QString)));
  connect(&player_, SIGNAL(Ready()), SLOT(PlayerReady()));
  connect(&player_, SIGNAL(AppStopped()), SLOT(PlayerAppStopped()));
  connect(&player_, SIGNAL(StateChanged(CastMediaPlayer::State)),
          SLOT(PlayerStateChanged(CastMediaPlayer::State)));
  connect(&player_, SIGNAL(TrackEnded()), SLOT(PlayerTrackEnded()));
  connect(&player_, SIGNAL(Error(QString)), SLOT(PlayerError(QString)));
}

ChromecastEngine::~ChromecastEngine() { items_->Remove(token_); }

void ChromecastEngine::SetDevice(const CastDevice& device) {
  const bool moved =
      device.address != device_.address || device.port != device_.port;
  device_ = device;
  if (!moved) return;

  // The token is only for the old address, and the connection's to it.
  PublishItems();
  if (channel_.is_open()) channel_.Close();
}

void ChromecastEngine::PublishItems() {
  QList<StreamItem> items;
  if (current_.id != 0) items << current_;
  items_->Set(token_, device_.address, items);
}

Song ChromecastEngine::SongForRequest(const MediaPlaybackRequest& req) const {
  // As RemoteEngine does: the engine interface only carries URLs.
  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (item && item->Url() == req.RequestUrl()) return item->Metadata();

  Song song;
  song.set_url(req.MediaUrl());
  return song;
}

StreamItem ChromecastEngine::MakeItem(const MediaPlaybackRequest& req,
                                      const Song& song, bool force_encode) {
  StreamSettings settings = settings_;
  settings.force_encode = force_encode;

  StreamItem item;
  item.id = next_item_id_++;
  item.req = req;
  item.song = song;
  item.plan = StreamPlanner::Plan(req, song, Caps(), settings);

  qLog(Debug) << "Cast item" << item.id << req.MediaUrl() << "->"
              << item.plan.mode << item.plan.mime_type << "("
              << item.plan.reason << ")";
  return item;
}

QUrl ChromecastEngine::UrlForItem(const StreamItem& item,
                                  qint64 start_ms) const {
  QUrl url = registry_->MediaBaseUrl(device_.address);
  if (url.isEmpty()) return url;
  url.setPath(
      QString("/s/%1/%2").arg(QString::fromLatin1(token_)).arg(item.id));
  if (start_ms > 0) url.setQuery(QString("t=%1").arg(start_ms));
  return url;
}

void ChromecastEngine::Connect() {
  if (channel_.is_open()) {
    if (!player_.is_ready()) player_.Launch();
    return;
  }
  qLog(Info) << "Connecting to" << device_;
  channel_.Open(device_.address, device_.port);
}

void ChromecastEngine::ChannelOpened() { player_.Launch(); }

void ChromecastEngine::ChannelClosed(const QString& reason) {
  qLog(Warning) << "Lost the connection to" << device_.name << ":" << reason;
  current_sent_ = false;
  if (state_ == Engine::Playing || state_ == Engine::Paused ||
      pending_.pending) {
    pending_ = PendingLoad();
    emit DeviceFailed();
  }
}

void ChromecastEngine::PlayerReady() {
  if (!pending_.pending) return;
  const PendingLoad pending = pending_;
  pending_ = PendingLoad();
  SendLoad(pending.start_ms, pending.playing);
}

void ChromecastEngine::PlayerAppStopped() {
  // Someone else is using the device now.
  current_sent_ = false;
  if (state_ == Engine::Playing || state_ == Engine::Paused) {
    qLog(Info) << device_.name << "is playing something else now";
    emit DeviceFailed();
  }
}

void ChromecastEngine::SendLoad(qint64 start_ms, bool playing) {
  if (current_.id == 0) return;
  if (!player_.is_ready()) {
    pending_.pending = true;
    pending_.start_ms = start_ms;
    pending_.playing = playing;
    Connect();
    return;
  }

  // Pipeline output can't be seeked into, so it gets a URL that starts at
  // the position instead.
  const bool pipeline = current_.plan.mode == StreamPlan::Pipeline;
  const QUrl url = UrlForItem(current_, pipeline ? start_ms : 0);
  if (url.isEmpty()) {
    FailItem(tr("%1 isn't on a network the network remote listens on")
                 .arg(device_.name));
    return;
  }

  CastMediaInfo info;
  info.url = url;
  info.content_type = current_.plan.mime_type;
  info.title = current_.song.PrettyTitle();
  info.artist = current_.song.artist();
  info.album = current_.song.album();
  if (current_.plan.length_nanosec > 0) {
    info.duration_ms = current_.plan.length_nanosec / kNsecPerMsec;
  }
  position_base_ms_ = pipeline ? start_ms : 0;
  player_.Load(info, pipeline ? 0 : start_ms, playing);
  current_sent_ = true;
}

void ChromecastEngine::SetState(Engine::State state) {
  if (state == state_) return;
  state_ = state;
  emit StateChanged(state);
}

void ChromecastEngine::FailItem(const QString& message) {
  const MediaPlaybackRequest req = current_.req;
  current_ = StreamItem();
  current_sent_ = false;
  PublishItems();
  SetState(Engine::Error);
  emit Error(message);
  emit InvalidMediaRequested(req);
}

bool ChromecastEngine::Load(const MediaPlaybackRequest& req,
                            Engine::TrackChangeFlags change,
                            bool force_stop_at_end, quint64 beginning_nanosec,
                            qint64 end_nanosec) {
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

bool ChromecastEngine::Play(quint64 offset_nanosec) {
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

void ChromecastEngine::Stop(bool) {
  if (current_sent_) player_.Stop();
  current_ = StreamItem();
  current_sent_ = false;
  pending_ = PendingLoad();
  position_base_ms_ = 0;
  playback_req_ = MediaPlaybackRequest();
  PublishItems();
  SetState(Engine::Empty);
}

void ChromecastEngine::Pause() {
  if (state_ != Engine::Playing) return;
  if (pending_.pending) {
    pending_.playing = false;
  } else {
    player_.Pause();
  }
  SetState(Engine::Paused);
}

void ChromecastEngine::Unpause() {
  if (state_ != Engine::Paused) return;
  if (pending_.pending) {
    pending_.playing = true;
  } else {
    player_.Play();
  }
  SetState(Engine::Playing);
}

void ChromecastEngine::Seek(quint64 offset_nanosec) {
  if (current_.id == 0) return;
  const qint64 position_ms = offset_nanosec / kNsecPerMsec;

  if (pending_.pending) {
    pending_.start_ms = position_ms;
    return;
  }
  if (!current_sent_) return;
  if (current_.plan.mode == StreamPlan::Direct) {
    player_.Seek(position_ms);
  } else if (current_.plan.length_nanosec > 0) {
    // A new URL that starts there.
    SendLoad(position_ms, state_ != Engine::Paused);
  }
}

qint64 ChromecastEngine::position_nanosec() const {
  qint64 ms;
  if (pending_.pending) {
    ms = pending_.start_ms;
  } else {
    ms = position_base_ms_ + player_.position_ms();
  }
  const qint64 nanosec = ms * kNsecPerMsec;
  const qint64 length = length_nanosec();
  return length > 0 ? qMin(nanosec, length) : nanosec;
}

qint64 ChromecastEngine::length_nanosec() const {
  if (current_.id == 0) return 0;
  if (current_.plan.length_nanosec > 0) return current_.plan.length_nanosec;
  return player_.duration_ms() * kNsecPerMsec;
}

void ChromecastEngine::SetVolumeSW(uint) {
  // Not before connecting; see the class comment.
  if (!player_.is_ready()) return;
  player_.SetVolume(volume_ / 100.0);
}

void ChromecastEngine::PlayerStateChanged(CastMediaPlayer::State state) {
  if (!current_sent_) return;
  switch (state) {
    case CastMediaPlayer::Buffering:
      SetState(Engine::Playing);
      break;
    case CastMediaPlayer::Playing:
      SetState(Engine::Playing);
      if (!valid_emitted_) {
        valid_emitted_ = true;
        emit ValidMediaRequested(current_.req);
      }
      break;
    case CastMediaPlayer::Paused:
      SetState(Engine::Paused);
      break;
    case CastMediaPlayer::Idle:
      // TrackEnded or Error says why.
      break;
  }
}

void ChromecastEngine::PlayerTrackEnded() {
  if (current_.id == 0) return;
  current_sent_ = false;
  SetState(Engine::Idle);
  emit TrackEnded();
}

void ChromecastEngine::PlayerError(const QString& message) {
  qLog(Warning) << device_.name << "error:" << message;
  if (current_.id == 0) return;
  if (!current_sent_) {
    // The receiver couldn't be launched for a load that's waiting for it.
    if (pending_.pending) {
      pending_ = PendingLoad();
      FailItem(message);
    }
    return;
  }

  // The device was sent the original but couldn't play it: try once more,
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

void ChromecastEngine::timerEvent(QTimerEvent* e) {
  if (e->timerId() != timer_id_) return;
  if (state_ != Engine::Playing) return;

  const qint64 length = length_nanosec();
  if (length <= 0) return;

  const qint64 fudge = (kTimerIntervalMsec + 100) * kNsecPerMsec;
  if (length - position_nanosec() < kPreloadGapNanosec + fudge) {
    EmitAboutToEnd();
  }
}
