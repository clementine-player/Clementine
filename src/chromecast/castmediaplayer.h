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

#ifndef CHROMECAST_CASTMEDIAPLAYER_H_
#define CHROMECAST_CASTMEDIAPLAYER_H_

#include <QElapsedTimer>
#include <QJsonObject>
#include <QObject>
#include <QUrl>

class CastChannel;

// What to play, and what the device shows while it plays.
struct CastMediaInfo {
  QUrl url;
  QString content_type;
  QString title;
  QString artist;
  QString album;
  QUrl image_url;
  // 0 if unknown.
  qint64 duration_ms = 0;
};

// Plays music on a Cast device with Google's Default Media Receiver, an app
// every Cast device has that plays a URL it's given.
//
// Launch() starts the app, or joins it if it's already running. Once Ready
// is emitted, Load() and the other media commands go to it. If the app goes
// away - another sender takes over the device, or someone stops it - this
// emits AppStopped and has to be launched again.
//
// Volume is the device's own.
class CastMediaPlayer : public QObject {
  Q_OBJECT

 public:
  static const char* kDefaultMediaReceiverAppId;
  static const char* kReceiverNamespace;
  static const char* kMediaNamespace;

  enum State { Idle, Buffering, Playing, Paused };
  Q_ENUM(State)

  // |channel| must outlive this.
  explicit CastMediaPlayer(CastChannel* channel, QObject* parent = nullptr);

  void Launch();
  // Stops the app on the device.
  void StopApp();
  bool is_ready() const { return !transport_id_.isEmpty(); }

  void Load(const CastMediaInfo& info, qint64 start_ms, bool autoplay);
  void Play();
  void Pause();
  void Stop();
  void Seek(qint64 position_ms);

  // 0.0 to 1.0.
  void SetVolume(double level);
  void SetMuted(bool muted);

  State state() const { return state_; }
  // Where the device last said it was, moved on by the time since then
  // while it's playing.
  qint64 position_ms() const;
  // 0 if the device doesn't know.
  qint64 duration_ms() const { return duration_ms_; }
  double volume() const { return volume_; }
  bool muted() const { return muted_; }

 signals:
  void Ready();
  void AppStopped();
  void StateChanged(CastMediaPlayer::State state);
  // The track played to the end.
  void TrackEnded();
  // The device couldn't launch the app or play the track.
  void Error(const QString& message);
  void VolumeChanged(double level, bool muted);

 private slots:
  void MessageReceived(const QString& name_space, const QString& source_id,
                       const QJsonObject& payload);

 private:
  void SendReceiver(QJsonObject payload);
  void SendMedia(QJsonObject payload);
  // A media command for the current track, if there is one.
  void SendMediaCommand(const QString& type, QJsonObject payload = {});

  void ReceiverStatus(const QJsonObject& status);
  void MediaStatus(const QJsonObject& status);
  void SetState(State state);
  void AppGone();

  CastChannel* channel_;
  int next_request_id_;
  bool launching_;

  QString session_id_;
  QString transport_id_;
  // 0 when nothing is loaded.
  int media_session_id_;
  // The LOAD in flight, if any; its MEDIA_STATUS sets media_session_id_.
  int load_request_id_;
  // The last track that went idle. Devices repeat its final status, which
  // mustn't end it twice.
  int idle_media_session_id_;

  State state_;
  qint64 position_ms_;
  QElapsedTimer position_updated_;
  double playback_rate_;
  qint64 duration_ms_;
  double volume_;
  bool muted_;
};

#endif  // CHROMECAST_CASTMEDIAPLAYER_H_
