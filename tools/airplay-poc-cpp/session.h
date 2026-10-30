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

#ifndef AIRPLAY_SESSION_H_
#define AIRPLAY_SESSION_H_

#include <QElapsedTimer>
#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <functional>
#include <memory>

#include "audiosource.h"
#include "crypto.h"
#include "discovery.h"
#include "rtspconnection.h"
#include "tlv8.h"

class QTcpSocket;
class QTimer;
class QUdpSocket;

namespace airplay {

// An AirPlay audio session: set up over RTSP (AirPlay 1, or AirPlay 2 with
// transient pairing), then 44.1 kHz 16-bit stereo over RTP until the source
// ends, with the timing and control channels the receiver expects. Runs on
// the event loop, without threads.
class Session : public QObject {
  Q_OBJECT

 public:
  struct Options {
    Version version = Version::kAirPlay2;
    QString pin = "3939";
    // Whether |pin| is the receiver's password rather than the fixed PIN of
    // transient pairing.
    bool has_password = false;
    // 0 to 100, or negative to leave the receiver's volume alone.
    int volume = -1;
    // What the receiver shows for this sender, for example when it asks
    // whether to allow it.
    QString name = "Clementine";
    // This sender's id, as hex, or empty for a random one each session.
    QByteArray device_id;
  };

  Session(const Options& options, std::unique_ptr<AudioSource> source,
          QObject* parent = nullptr);
  ~Session();

  void Start(const QHostAddress& host, quint16 port);

 signals:
  // The source ended, it was all played out and the session was torn down.
  void Finished();
  void Failed(const QString& error);

 private slots:
  void Connected();
  void Fail(const QString& error);
  void TimingReadyRead();
  void ControlReadyRead();
  void SendSync();
  void SendFeedback();
  void SendAudio();
  void ConnectEventChannel();
  void EventConnected();
  void EventError();
  void EventReadyRead();

 private:
  // False, after failing the session, unless |response| is 200.
  bool Check(const RtspConnection::Response& response, const QString& what);
  Headers SessionHeaders() const;
  QString PairingError(int code, const tlv8::Items& items) const;

  void GetInfo();
  void SetupAirPlay1();
  void Pair();
  void PairM3(const QByteArray& response);
  void SetupAirPlay2Session();
  void SetupAirPlay2Stream();
  void StartStreaming(quint16 data_port, quint16 control_port);
  // Calls |done| once the receiver has answered.
  void SetVolume(std::function<void()> done);
  void StartAudio();
  void SendPacket();
  void Finish();

  quint32 RtpTime() const;

  Options options_;
  std::unique_ptr<AudioSource> source_;
  QHostAddress host_;
  quint16 port_ = 0;

  RtspConnection* connection_;
  QUdpSocket* timing_;
  QUdpSocket* control_;
  QUdpSocket* audio_;
  QTimer* sync_timer_;
  QTimer* feedback_timer_;
  QTimer* audio_timer_;
  bool done_ = false;
  bool paired_ = false;

  QString rtsp_session_;
  QByteArray shared_key_;
  // AirPlay 2 encrypts each audio packet with this, which it sends in SETUP.
  QByteArray audio_key_;
  quint64 audio_nonce_ = 0;

  // The event channel AirPlay 2 receivers send requests on.
  QTcpSocket* event_socket_ = nullptr;
  std::unique_ptr<FrameCipher> event_cipher_;
  QByteArray event_buffer_;
  quint16 event_port_ = 0;
  int event_attempts_ = 0;

  quint16 data_port_ = 0;
  quint16 remote_control_port_ = 0;
  quint16 seq_ = 0;
  quint64 start_ts_ = 0;
  quint64 head_ts_ = 0;
  bool first_packet_ = true;
  bool first_sync_ = true;
  // Recent packets, for resend requests.
  QHash<quint16, QByteArray> backlog_;

  QElapsedTimer started_;
  QElapsedTimer last_report_;
  qint64 sent_frames_ = 0;
  // Silence sent after the source ended.
  qint64 padding_ = 0;
};

}  // namespace airplay

#endif  // AIRPLAY_SESSION_H_
