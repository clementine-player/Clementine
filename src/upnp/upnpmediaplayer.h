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

#ifndef UPNP_UPNPMEDIAPLAYER_H_
#define UPNP_UPNPMEDIAPLAYER_H_

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QUrl>
#include <memory>

#include "upnpdevice.h"
#include "upnpsoap.h"

class EventCallbackTable;
class QNetworkAccessManager;
class QTimer;
class UpnpSubscription;

// What to play, and what the renderer shows while it plays.
struct UpnpMediaInfo {
  QUrl url;
  QString content_type;
  QString title;
  QString artist;
  QString album;
  QUrl image_url;
  // 0 if unknown.
  qint64 duration_ms = 0;
};

// Plays one URL at a time on a UPnP MediaRenderer, with its AVTransport,
// and queues the next where the renderer has SetNextAVTransportURI.
//
// Actions are sent one at a time, in the order they're asked for. While
// something's loaded, the renderer is asked for its state and position every
// kPollIntervalMsec, which is how this finds out that the track ended, that
// it couldn't be played, or that something else is playing on it now.
//
// Once subscribed, the renderer also sends events about its AVTransport,
// which say the same as the polls do, straight away. While they come, it's
// only polled every kEventPollIntervalMsec, for its position and in case
// they stop.
//
// Volume is the renderer's own, through RenderingControl, or its group's
// where it has a GroupRenderingControl (Sonos). Its events say when that
// changes, from anywhere.
class UpnpMediaPlayer : public QObject {
  Q_OBJECT

 public:
  static const int kPollIntervalMsec;
  static const int kEventPollIntervalMsec;
  // A track that stops this close to its end finished.
  static const int kEndMarginMsec;

  enum State { Idle, Buffering, Playing, Paused };
  Q_ENUM(State)

  explicit UpnpMediaPlayer(QObject* parent = nullptr);
  ~UpnpMediaPlayer();

  // Where its services are.
  void SetDevice(const UpnpDevice& device);
  // Where the media server takes events, for Subscribe.
  void SetEventCallbacks(std::shared_ptr<EventCallbackTable> callbacks);

  // Subscribes to the renderer's events, sent to the media server at |base|,
  // if it isn't already. They're kept until Stop or Unsubscribe.
  void Subscribe(const QUrl& base);
  void Unsubscribe();

  void Load(const UpnpMediaInfo& info, qint64 start_ms, bool autoplay);
  void Play();
  void Pause();
  void Stop();
  void Seek(qint64 position_ms);
  // Has the renderer play |info| when the loaded track ends, without a gap.
  // Nothing happens if it can't; see can_queue_next.
  void SetNext(const UpnpMediaInfo& info);
  // 0 to 100.
  void SetVolume(int volume);

  bool is_loaded() const { return loaded_; }
  // False once the renderer has said it doesn't have SetNextAVTransportURI.
  bool can_queue_next() const { return !next_unsupported_; }
  State state() const { return state_; }
  // Where the renderer last said it was, moved on by the time since then
  // while it's playing.
  qint64 position_ms() const;
  // 0 if unknown.
  qint64 duration_ms() const { return duration_ms_; }
  int poll_interval_msec() const;

  // The DIDL-Lite that describes |info| to the renderer.
  static QString Didl(const UpnpMediaInfo& info);
  // "H:MM:SS", as AVTransport has times.
  static QString FormatTime(qint64 ms);
  // -1 if |time| isn't one, eg. "NOT_IMPLEMENTED".
  static qint64 ParseTime(const QString& time);

 signals:
  void StateChanged(UpnpMediaPlayer::State state);
  // The track played to the end.
  void TrackEnded();
  // The track played to the end, and the renderer went on to what SetNext
  // queued, which is what's loaded now.
  void NextStarted();
  // The renderer couldn't play the track.
  void Error(const QString& message);
  // It can't be reached, or something else is playing on it now. Nothing is
  // loaded any more.
  void Lost(const QString& reason);
  // The renderer's volume, 0 to 100, as it was when subscribing and whenever
  // it changes.
  void RendererVolumeChanged(int volume);

 protected:
  // Sends |action|, then calls Finished with the answer. Tests answer it
  // themselves.
  virtual void Send(const UpnpSoap::Action& action);
  void Finished(const UpnpSoap::Response& response);
  // From the subscriptions; tests send them themselves.
  void TransportEvent(const QMap<QString, QString>& variables);
  void VolumeEvent(const QMap<QString, QString>& variables);

 private slots:
  void Poll();

 private:
  enum Kind {
    SetUri,
    LoadSeek,
    LoadPlay,
    SetNextUri,
    Command,
    Volume,
    PollPosition,
    PollTransport,
  };
  struct Request {
    Kind kind;
    // Load's; replies about something older are ignored.
    int generation;
    UpnpSoap::Action action;
  };

  void Enqueue(Kind kind, const UpnpService& service, const QString& name,
               UpnpSoap::Args args = {});
  void SendNext();
  bool Queued(Kind kind) const;
  // Forgets what's loaded, and anything that was to be sent about it.
  void Unload();

  void PositionInfo(const QMap<QString, QString>& values);
  void TransportInfo(const QMap<QString, QString>& values);
  void SetState(State state);
  // A subscription to |service|'s events, or null if it has none.
  UpnpSubscription* MakeSubscription(const UpnpService& service,
                                     const QUrl& base);

  QNetworkAccessManager* network_;
  QTimer* poll_timer_;
  UpnpDevice device_;

  QList<Request> queue_;
  bool in_flight_;
  Request current_;

  int generation_;
  bool loaded_;
  QString uri_;
  // Whether the renderer's played or paused this track yet.
  bool started_;
  // Whether it's been told to play, and has said it will.
  bool play_sent_;
  bool want_play_;
  // Polls since then that found it stopped without having started.
  int stopped_polls_;

  State state_;
  qint64 position_ms_;
  QElapsedTimer position_updated_;
  // Where it was when last seen playing.
  qint64 last_playing_position_ms_;
  // Where to seek to once it's playing or paused, or -1. Some renderers,
  // like Rygel, can't seek until then, so seeking as it's loaded fails.
  qint64 pending_seek_ms_;
  qint64 duration_ms_;

  // What SetNext queued, until the renderer plays it.
  QString next_uri_;
  qint64 next_duration_ms_;
  bool next_unsupported_;

  std::shared_ptr<EventCallbackTable> callbacks_;
  UpnpSubscription* transport_events_;
  UpnpSubscription* volume_events_;
};

#endif  // UPNP_UPNPMEDIAPLAYER_H_
