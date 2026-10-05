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

#ifndef CHROMECAST_CHROMECASTENGINE_H_
#define CHROMECAST_CHROMECASTENGINE_H_

#include "castchannel.h"
#include "castdevice.h"
#include "castmediaplayer.h"
#include "engines/enginebase.h"
#include "networkremote/streaming/streamitemtable.h"
#include "networkremote/streaming/streamplanner.h"

class Application;
class RendererRegistry;

// The engine for one Cast device: an output of the EngineRouter, like a
// RemoteEngine, that plays through the device's Default Media Receiver.
//
// Media is served by the network remote's MediaHttpServer, under a token of
// this engine's that only the device's address may use. The connection to
// the device is made when the engine is first asked to play, and kept.
//
// Clementine's volume slider sets the device's volume, but only once
// connected: switching to the device leaves its volume as it was, rather
// than jumping to wherever the slider is.
class ChromecastEngine : public Engine::Base {
  Q_OBJECT

 public:
  // The formats every Cast device plays.
  static RendererCaps Caps();

  ChromecastEngine(Application* app, const CastDevice& device,
                   const RendererRegistry* registry, StreamItemTable* items,
                   QObject* parent = nullptr);
  ~ChromecastEngine();

  const CastDevice& device() const { return device_; }
  // When the device's name or address changes.
  void SetDevice(const CastDevice& device);

  // Engine::Base
  bool Init() override { return true; }
  void StartPreloading(const MediaPlaybackRequest&, bool, qint64,
                       qint64) override {}
  bool Load(const MediaPlaybackRequest& req, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec) override;
  bool Play(quint64 offset_nanosec) override;
  void Stop(bool stop_after = false) override;
  void Pause() override;
  void Unpause() override;
  void Seek(quint64 offset_nanosec) override;
  Engine::State state() const override { return state_; }
  qint64 position_nanosec() const override;
  qint64 length_nanosec() const override;

 signals:
  // The connection to the device was lost, or another sender took it over.
  void DeviceFailed();

 protected:
  void SetVolumeSW(uint percent) override;
  void timerEvent(QTimerEvent* e) override;

 private slots:
  void ChannelOpened();
  void ChannelClosed(const QString& reason);
  void PlayerReady();
  void PlayerAppStopped();
  void PlayerStateChanged(CastMediaPlayer::State state);
  void PlayerTrackEnded();
  void PlayerError(const QString& message);

 private:
  // What to load once the device is ready.
  struct PendingLoad {
    bool pending = false;
    qint64 start_ms = 0;
    bool playing = false;
  };

  StreamItem MakeItem(const MediaPlaybackRequest& req, const Song& song,
                      bool force_encode);
  Song SongForRequest(const MediaPlaybackRequest& req) const;
  QUrl UrlForItem(const StreamItem& item, qint64 start_ms) const;
  void PublishItems();

  // Connects to the device and launches the receiver, if that isn't done.
  void Connect();
  void SendLoad(qint64 start_ms, bool playing);
  void SetState(Engine::State state);
  void FailItem(const QString& message);

  Application* app_;
  CastDevice device_;
  const RendererRegistry* registry_;
  StreamItemTable* items_;
  StreamSettings settings_;
  const QByteArray token_;
  int next_item_id_;

  CastChannel channel_;
  CastMediaPlayer player_;

  StreamItem current_;
  // Whether current_ has been sent to the device.
  bool current_sent_;
  bool current_retried_;
  bool valid_emitted_;
  PendingLoad pending_;
  // A Pipeline item sought into starts at this position, and the device
  // counts from 0.
  qint64 position_base_ms_;

  Engine::State state_;
  int timer_id_;
};

#endif  // CHROMECAST_CHROMECASTENGINE_H_
