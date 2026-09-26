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

#include "enginerouter.h"

#include <QTimer>

#include "core/logging.h"
#include "gstengine.h"

namespace {
const int kPauseRetryMsec = 50;
const int kMaxPauseAttempts = 100;
}  // namespace

EngineRouter::EngineRouter(Engine::Base* local)
    : local_(local),
      active_(local),
      force_stop_at_end_(false),
      pause_when_playing_(false),
      pause_attempts_(0) {
  ConnectEngine(local_.get());
  if (local_engine()) {
    connect(local_.get(), SIGNAL(Initialised()), SIGNAL(Initialised()));
  }
}

GstEngine* EngineRouter::local_engine() const {
  return qobject_cast<GstEngine*>(local_.get());
}

EngineRouter::~EngineRouter() {
  for (Engine::Base* remote : remotes_) {
    disconnect(remote, nullptr, this, nullptr);
  }
}

bool EngineRouter::is_local() const { return active_ == local_.get(); }

void EngineRouter::ConnectEngine(Engine::Base* engine) {
  connect(engine, SIGNAL(TrackAboutToEnd()), SLOT(ForwardTrackAboutToEnd()));
  connect(engine, SIGNAL(TrackEnded()), SLOT(ForwardTrackEnded()));
  connect(engine, SIGNAL(FadeoutFinishedSignal()),
          SLOT(ForwardFadeoutFinished()));
  connect(engine, SIGNAL(StatusText(QString)),
          SLOT(ForwardStatusText(QString)));
  connect(engine, SIGNAL(Error(QString)), SLOT(ForwardError(QString)));
  connect(engine, SIGNAL(InvalidMediaRequested(MediaPlaybackRequest)),
          SLOT(ForwardInvalidMediaRequested(MediaPlaybackRequest)));
  connect(engine, SIGNAL(ValidMediaRequested(MediaPlaybackRequest)),
          SLOT(ForwardValidMediaRequested(MediaPlaybackRequest)));
  connect(engine, SIGNAL(MetaData(Engine::SimpleMetaBundle)),
          SLOT(ForwardMetaData(Engine::SimpleMetaBundle)));
  connect(engine, SIGNAL(StateChanged(Engine::State)),
          SLOT(ForwardStateChanged(Engine::State)));
}

void EngineRouter::AddOutput(Engine::Base* engine) {
  if (remotes_.contains(engine)) return;
  remotes_ << engine;
  ConnectEngine(engine);
  connect(engine, SIGNAL(destroyed(QObject*)), SLOT(OutputDestroyed(QObject*)));

  engine->ReloadSettings();
  engine->SetVolume(volume_);
  emit OutputsChanged();
}

void EngineRouter::RemoveOutput(Engine::Base* engine) {
  if (!remotes_.removeOne(engine)) return;
  disconnect(engine, nullptr, this, nullptr);

  if (active_ == engine) {
    // Don't start playing out loud on this computer; carry on paused.
    const Engine::State state = engine->state();
    const qint64 position = engine->position_nanosec();
    qLog(Info) << "Active output went away, falling back to this computer";

    active_ = local_.get();
    UpdateFadeSettings();
    if (state == Engine::Playing || state == Engine::Paused) {
      local_->Load(playback_req_, Engine::Manual, force_stop_at_end_,
                   beginning_nanosec_, end_nanosec_);
      pause_when_playing_ = true;
      local_->Play(position);
    } else {
      emit StateChanged(local_->state());
    }
  }
  emit OutputsChanged();
}

void EngineRouter::OutputDestroyed(QObject* object) {
  // Only the QObject part is left, so compare pointers without calling
  // anything on it.
  for (Engine::Base* remote : remotes_) {
    if (static_cast<QObject*>(remote) == object) {
      remotes_.removeOne(remote);
      if (active_ == remote) {
        active_ = local_.get();
        UpdateFadeSettings();
        emit StateChanged(Engine::Empty);
      }
      emit OutputsChanged();
      return;
    }
  }
}

void EngineRouter::SetOutput(Engine::Base* engine) {
  if (engine == active_) return;
  if (engine != local_.get() && !remotes_.contains(engine)) {
    qLog(Warning) << "Asked to use an output that isn't registered";
    return;
  }

  Engine::Base* old = active_;
  const Engine::State state = old->state();
  const qint64 position = old->position_nanosec();

  // Switch first, so the old engine's StateChanged(Empty) from Stop() is
  // ignored rather than reaching the Player.
  active_ = engine;
  UpdateFadeSettings();
  engine->SetVolume(volume_);
  old->Stop();

  if ((state == Engine::Playing || state == Engine::Paused) &&
      !playback_req_.MediaUrl().isEmpty()) {
    engine->Load(playback_req_, Engine::Manual, force_stop_at_end_,
                 beginning_nanosec_, end_nanosec_);
    pause_when_playing_ = state == Engine::Paused;
    engine->Play(position);
  }

  emit OutputsChanged();
}

void EngineRouter::SetLocalOutput() { SetOutput(local_.get()); }

void EngineRouter::UpdateFadeSettings() {
  Engine::Base::ReloadSettings();
  if (!is_local()) {
    fadeout_enabled_ = false;
    crossfade_enabled_ = false;
    autocrossfade_enabled_ = false;
    fadeout_pause_enabled_ = false;
  }
}

bool EngineRouter::Init() { return local_->Init(); }

void EngineRouter::StartPreloading(const MediaPlaybackRequest& req,
                                   bool force_stop_at_end,
                                   qint64 beginning_nanosec,
                                   qint64 end_nanosec) {
  active_->StartPreloading(req, force_stop_at_end, beginning_nanosec,
                           end_nanosec);
}

bool EngineRouter::Load(const MediaPlaybackRequest& req,
                        Engine::TrackChangeFlags change, bool force_stop_at_end,
                        quint64 beginning_nanosec, qint64 end_nanosec) {
  Engine::Base::Load(req, change, force_stop_at_end, beginning_nanosec,
                     end_nanosec);
  force_stop_at_end_ = force_stop_at_end;
  pause_when_playing_ = false;
  return active_->Load(req, change, force_stop_at_end, beginning_nanosec,
                       end_nanosec);
}

void EngineRouter::RefreshMarkers(quint64 beginning_nanosec,
                                  qint64 end_nanosec) {
  Engine::Base::RefreshMarkers(beginning_nanosec, end_nanosec);
  active_->RefreshMarkers(beginning_nanosec, end_nanosec);
}

bool EngineRouter::Play(quint64 offset_nanosec) {
  return active_->Play(offset_nanosec);
}

void EngineRouter::Stop(bool stop_after) {
  pause_when_playing_ = false;
  active_->Stop(stop_after);
}

void EngineRouter::Pause() { active_->Pause(); }

void EngineRouter::Unpause() {
  pause_when_playing_ = false;
  active_->Unpause();
}

void EngineRouter::Seek(quint64 offset_nanosec) {
  active_->Seek(offset_nanosec);
}

// Background streams are ambience sounds, not part of the playlist, so they
// always play here.
int EngineRouter::AddBackgroundStream(const QUrl& url) {
  return local_->AddBackgroundStream(url);
}

void EngineRouter::StopBackgroundStream(int id) {
  local_->StopBackgroundStream(id);
}

void EngineRouter::SetBackgroundStreamVolume(int id, int volume) {
  local_->SetBackgroundStreamVolume(id, volume);
}

Engine::State EngineRouter::state() const { return active_->state(); }

qint64 EngineRouter::position_nanosec() const {
  return active_->position_nanosec();
}

qint64 EngineRouter::length_nanosec() const {
  return active_->length_nanosec();
}

const Engine::Scope& EngineRouter::scope(int chunk_length) {
  return active_->scope(chunk_length);
}

void EngineRouter::ReloadSettings() {
  local_->ReloadSettings();
  for (Engine::Base* remote : remotes_) remote->ReloadSettings();
  UpdateFadeSettings();
}

void EngineRouter::SetEqualizerEnabled(bool enabled) {
  local_->SetEqualizerEnabled(enabled);
  for (Engine::Base* remote : remotes_) remote->SetEqualizerEnabled(enabled);
}

void EngineRouter::SetEqualizerParameters(int preamp,
                                          const QList<int>& band_gains) {
  local_->SetEqualizerParameters(preamp, band_gains);
  for (Engine::Base* remote : remotes_) {
    remote->SetEqualizerParameters(preamp, band_gains);
  }
}

void EngineRouter::SetStereoBalance(float value) {
  local_->SetStereoBalance(value);
  for (Engine::Base* remote : remotes_) remote->SetStereoBalance(value);
}

void EngineRouter::SetVolumeSW(uint) {
  // Each engine applies its own curve, so hand it the linear volume.
  active_->SetVolume(volume_);
}

bool EngineRouter::IsActive(QObject* sender) const {
  return sender == static_cast<QObject*>(active_);
}

void EngineRouter::ForwardTrackAboutToEnd() {
  if (IsActive(sender())) emit TrackAboutToEnd();
}

void EngineRouter::ForwardTrackEnded() {
  if (IsActive(sender())) emit TrackEnded();
}

void EngineRouter::ForwardFadeoutFinished() {
  if (IsActive(sender())) emit FadeoutFinishedSignal();
}

void EngineRouter::ForwardStatusText(const QString& text) {
  if (IsActive(sender())) emit StatusText(text);
}

void EngineRouter::ForwardError(const QString& message) {
  if (IsActive(sender())) emit Error(message);
}

void EngineRouter::ForwardInvalidMediaRequested(
    const MediaPlaybackRequest& req) {
  if (IsActive(sender())) emit InvalidMediaRequested(req);
}

void EngineRouter::ForwardValidMediaRequested(const MediaPlaybackRequest& req) {
  if (IsActive(sender())) emit ValidMediaRequested(req);
}

void EngineRouter::ForwardMetaData(const Engine::SimpleMetaBundle& bundle) {
  if (IsActive(sender())) emit MetaData(bundle);
}

void EngineRouter::ForwardStateChanged(Engine::State state) {
  if (!IsActive(sender())) return;

  emit StateChanged(state);
  if (pause_when_playing_ && state == Engine::Playing) {
    pause_attempts_ = 0;
    PauseWhenPlaying();
  }
}

void EngineRouter::PauseWhenPlaying() {
  if (!pause_when_playing_) return;

  // GstEngine reports Playing before its pipeline has got there, and ignores
  // Pause() until it has.
  if (active_->state() == Engine::Playing) {
    pause_when_playing_ = false;
    active_->Pause();
  } else if (++pause_attempts_ < kMaxPauseAttempts) {
    QTimer::singleShot(kPauseRetryMsec, this, SLOT(PauseWhenPlaying()));
  } else {
    pause_when_playing_ = false;
  }
}
