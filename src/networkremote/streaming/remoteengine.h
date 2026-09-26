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

#ifndef NETWORKREMOTE_STREAMING_REMOTEENGINE_H_
#define NETWORKREMOTE_STREAMING_REMOTEENGINE_H_

#include <QElapsedTimer>
#include <QHostAddress>

#include "core/song.h"
#include "engines/enginebase.h"
#include "remotecontrolmessages.pb.h"
#include "streamitemtable.h"
#include "streamplanner.h"

class Application;

// Where a renderer's control connection is. RemoteClients live on the
// network remote's thread, so engines on the main thread only keep these
// values and talk to the client through signals.
struct RendererEndpoint {
  int client_id = 0;
  // The address and port the renderer connected to; media URLs use them.
  QHostAddress local_address;
  quint16 local_port = 0;
  // The renderer's address; only it may fetch its media URLs.
  QHostAddress peer_address;
};

// The engine for one connected renderer. It turns engine calls into RENDER_*
// commands on the renderer's control connection, and the renderer's reports
// back into engine signals.
//
// See docs/design/remote-streaming.md section 4.2.
class RemoteEngine : public Engine::Base {
  Q_OBJECT

 public:
  RemoteEngine(Application* app, const RendererEndpoint& endpoint,
               const cpb::remote::RendererCapabilities& caps,
               StreamItemTable* items, QObject* parent = nullptr);
  ~RemoteEngine();

  const QString& renderer_id() const { return renderer_id_; }
  const QString& display_name() const { return display_name_; }
  int client_id() const { return endpoint_.client_id; }

  // Handles a RENDERER_* message from this renderer.
  void HandleMessage(const cpb::remote::Message& msg);

  // Engine::Base
  bool Init() { return true; }
  void StartPreloading(const MediaPlaybackRequest& req, bool force_stop_at_end,
                       qint64 beginning_nanosec, qint64 end_nanosec);
  bool Load(const MediaPlaybackRequest& req, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec);
  bool Play(quint64 offset_nanosec);
  void Stop(bool stop_after = false);
  void Pause();
  void Unpause();
  void Seek(quint64 offset_nanosec);

  Engine::State state() const { return state_; }
  qint64 position_nanosec() const;
  qint64 length_nanosec() const;

 signals:
  // The renderer reported an error it can't recover from.
  void RendererFailed();
  // A serialized cpb::remote::Message for the renderer's connection.
  void SendToClient(int client_id, const QByteArray& data);

 protected:
  void SetVolumeSW(uint percent);
  void timerEvent(QTimerEvent* e);

 private:
  StreamItem MakeItem(const MediaPlaybackRequest& req, const Song& song,
                      bool force_encode);
  Song SongForRequest(const MediaPlaybackRequest& req, bool next) const;
  QString UrlForItem(const StreamItem& item, qint64 start_ms = 0) const;
  void FillRenderItem(const StreamItem& item,
                      cpb::remote::RenderItem* pb) const;
  void SendLoad(const StreamItem& item, qint64 start_ms, bool playing);
  void Send(cpb::remote::MsgType type, cpb::remote::Message* msg = nullptr);
  void SetState(Engine::State state);
  // Lets MediaHttpServer serve exactly the current and preloaded items.
  void PublishItems();
  void FailItem(const QString& message);

  void HandleStatus(const cpb::remote::RendererStatus& status);
  void HandleTrackEnded(int item_id);
  void HandleError(const cpb::remote::RendererError& error);

  Application* app_;
  RendererEndpoint endpoint_;
  StreamItemTable* items_;
  QString renderer_id_;
  QString display_name_;
  RendererCaps caps_;
  StreamSettings settings_;
  // Secret part of every media URL for this renderer.
  QByteArray token_;

  int next_item_id_;
  StreamItem current_;
  // Whether RENDER_LOAD has been sent for current_.
  bool current_sent_;
  bool current_retried_;
  StreamItem preloaded_;

  Engine::State state_;
  bool valid_emitted_;
  // Last position the renderer reported, and when.
  qint64 position_ms_;
  QElapsedTimer position_clock_;

  int timer_id_;
};

#endif  // NETWORKREMOTE_STREAMING_REMOTEENGINE_H_
