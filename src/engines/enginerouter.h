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

#ifndef ENGINES_ENGINEROUTER_H_
#define ENGINES_ENGINEROUTER_H_

#include <QList>
#include <memory>

#include "enginebase.h"

class GstEngine;

// The engine the Player talks to. It owns the local GstEngine and forwards
// every call to whichever output is active: the local engine, or an engine
// for a remote renderer. Only the active engine's signals are re-emitted, so
// everything connected to Player::engine() keeps working when the output
// changes.
//
// See docs/design/remote-streaming.md section 4.1.
class EngineRouter : public Engine::Base {
  Q_OBJECT

 public:
  // Takes ownership of |local|, normally a GstEngine.
  explicit EngineRouter(Engine::Base* local);
  ~EngineRouter();

  // The local engine, or nullptr if it isn't a GstEngine (in tests).
  GstEngine* local_engine() const;
  Engine::Base* active_engine() const { return active_; }
  bool is_local() const;

  // Makes |engine| available as an output. It isn't owned; it is removed
  // when destroyed.
  void AddOutput(Engine::Base* engine);
  // If |engine| is active, playback falls back to the local engine, paused.
  void RemoveOutput(Engine::Base* engine);

  // Moves playback to |engine|, keeping the position and play/pause state.
  // |engine| must be the local engine or one passed to AddOutput.
  void SetOutput(Engine::Base* engine);
  void SetLocalOutput();

  // Engine::Base
  bool Init();
  void StartPreloading(const MediaPlaybackRequest& req, bool force_stop_at_end,
                       qint64 beginning_nanosec, qint64 end_nanosec);
  bool Load(const MediaPlaybackRequest& req, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec);
  void RefreshMarkers(quint64 beginning_nanosec, qint64 end_nanosec);
  bool Play(quint64 offset_nanosec);
  void Stop(bool stop_after = false);
  void Pause();
  void Unpause();
  void Seek(quint64 offset_nanosec);

  int AddBackgroundStream(const QUrl& url);
  void StopBackgroundStream(int id);
  void SetBackgroundStreamVolume(int id, int volume);

  Engine::State state() const;
  qint64 position_nanosec() const;
  qint64 length_nanosec() const;
  const Engine::Scope& scope(int chunk_length);

 signals:
  // Forwarded from the local engine; see GstEngine::Initialised.
  void Initialised();
  // The active output changed, or an output was added or removed.
  void OutputsChanged();

 public slots:
  void ReloadSettings();
  void SetEqualizerEnabled(bool enabled);
  void SetEqualizerParameters(int preamp, const QList<int>& band_gains);
  void SetStereoBalance(float value);

 protected:
  void SetVolumeSW(uint percent);

 private slots:
  void OutputDestroyed(QObject* object);

  void ForwardTrackAboutToEnd();
  void ForwardTrackEnded();
  void ForwardFadeoutFinished();
  void ForwardStatusText(const QString& text);
  void ForwardError(const QString& message);
  void ForwardInvalidMediaRequested(const MediaPlaybackRequest& req);
  void ForwardValidMediaRequested(const MediaPlaybackRequest& req);
  void ForwardMetaData(const Engine::SimpleMetaBundle& bundle);
  void ForwardStateChanged(Engine::State state);
  void PauseWhenPlaying();

 private:
  void ConnectEngine(Engine::Base* engine);
  bool IsActive(QObject* sender) const;
  // Crossfades and fade-outs need two decoders mixed in one place, which a
  // remote renderer doesn't have, so they're only on for the local engine.
  void UpdateFadeSettings();

  std::unique_ptr<Engine::Base> local_;
  QList<Engine::Base*> remotes_;
  Engine::Base* active_;

  bool force_stop_at_end_;
  // Set while handing over paused playback: the new engine has to start
  // playing to seek, and is paused as soon as it's really playing.
  bool pause_when_playing_;
  int pause_attempts_;
};

#endif  // ENGINES_ENGINEROUTER_H_
