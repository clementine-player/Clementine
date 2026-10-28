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

#ifndef UPNP_UPNPENGINE_H_
#define UPNP_UPNPENGINE_H_

#include "engines/enginebase.h"
#include "networkremote/streaming/streamitemtable.h"
#include "networkremote/streaming/streamplanner.h"
#include "upnpdevice.h"
#include "upnpmediaplayer.h"

class Application;
class RendererRegistry;

// The engine for one UPnP MediaRenderer: an output of the EngineRouter, like
// a ChromecastEngine, that tells the renderer to play URLs with its
// AVTransport. Each track is planned against the formats the renderer's
// ConnectionManager lists. Where the renderer has SetNextAVTransportURI,
// the next track is queued on it before this one ends, for gapless playback.
//
// Media is served by the network remote's MediaHttpServer, under a token of
// this engine's that only the renderer's address may use.
//
// Clementine's volume slider sets the renderer's volume, but only once this
// has played on it: switching to it leaves its volume as it was, rather than
// jumping to wherever the slider is.
class UpnpEngine : public Engine::Base {
  Q_OBJECT

 public:
  UpnpEngine(Application* app, const UpnpDevice& device,
             const RendererRegistry* registry, StreamItemTable* items,
             QObject* parent = nullptr);
  ~UpnpEngine();

  const UpnpDevice& device() const { return device_; }
  // When its name, address or services change.
  void SetDevice(const UpnpDevice& device);

  // Engine::Base
  bool Init() override { return true; }
  void StartPreloading(const MediaPlaybackRequest& req, bool force_stop_at_end,
                       qint64 beginning_nanosec, qint64 end_nanosec) override;
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
  QString output_name() const override { return device_.name; }
  QString output_icon() const override;
  QHostAddress output_address() const override { return device_.address; }

 signals:
  // The renderer can't be reached, something else is playing on it now, or
  // media can't be served to it.
  void DeviceFailed();

 protected:
  void SetVolumeSW(uint percent) override;
  void timerEvent(QTimerEvent* e) override;

 private slots:
  void PlayerStateChanged(UpnpMediaPlayer::State state);
  void PlayerTrackEnded();
  void PlayerNextStarted();
  void PlayerError(const QString& message);
  void PlayerLost(const QString& reason);

 private:
  StreamItem MakeItem(const MediaPlaybackRequest& req, const Song& song,
                      bool force_encode);
  // The Player's current song, or with |next| the one it's preloading.
  Song SongForRequest(const MediaPlaybackRequest& req, bool next) const;
  QUrl UrlForItem(const StreamItem& item, qint64 start_ms) const;
  UpnpMediaInfo InfoForItem(const StreamItem& item, const QUrl& url) const;
  void PublishItems();

  void SendLoad(qint64 start_ms, bool playing);
  void SetState(Engine::State state);
  void FailItem(const QString& message);

  Application* app_;
  UpnpDevice device_;
  const RendererRegistry* registry_;
  StreamItemTable* items_;
  StreamSettings settings_;
  const QByteArray token_;
  int next_item_id_;

  UpnpMediaPlayer player_;
  // Whether this has played on the renderer since it was chosen; see the
  // class comment.
  bool in_control_;

  StreamItem current_;
  // Queued on the renderer to follow current_.
  StreamItem preloaded_;
  // Whether current_ has been sent to the renderer.
  bool current_sent_;
  bool current_retried_;
  bool valid_emitted_;
  // A Pipeline item sought into starts at this position, and the renderer
  // counts from 0.
  qint64 position_base_ms_;

  Engine::State state_;
  int timer_id_;
};

#endif  // UPNP_UPNPENGINE_H_
