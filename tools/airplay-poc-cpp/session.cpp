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

#include "session.h"

#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUuid>
#include <QtEndian>
#include <chrono>

#include "bplist.h"
#include "bytes.h"
#include "logging.h"
#include "srp.h"

namespace airplay {

namespace {

const int kFramesPerPacket = 352;
// Receivers play this many frames behind the RTP timestamps (1.5 s).
const quint32 kLatency = 22050 + kSampleRate;
const int kBacklog = 1000;
const int kEventAttempts = 5;
// A receiver may ask its user whether to allow a new sender before it answers
// the first request that needs it (pair-pin-start, or ANNOUNCE for AirPlay
// 1), and a Mac that has just woken can be slow too.
const int kApprovalTimeoutMsec = 30000;

quint64 NtpNow() {
  using namespace std::chrono;
  const quint64 us =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();
  const quint64 secs = us / 1000000 + 0x83AA7E80;
  const quint64 frac = ((us % 1000000) << 32) / 1000000;
  return (secs << 32) | frac;
}

quint64 NtpToTimestamp(quint64 ntp) {
  return ((ntp >> 16) * kSampleRate) >> 16;
}

quint64 TimestampToNtp(quint64 ts) { return ((ts << 16) / kSampleRate) << 16; }

class BitWriter {
 public:
  void Put(quint32 value, int bits) {
    acc_ = (acc_ << bits) | (value & ((quint64(1) << bits) - 1));
    bits_ += bits;
    while (bits_ >= 8) {
      bits_ -= 8;
      out_.append(char(acc_ >> bits_));
    }
  }

  QByteArray Finish() {
    if (bits_ > 0) out_.append(char(acc_ << (8 - bits_)));
    return out_;
  }

 private:
  QByteArray out_;
  quint64 acc_ = 0;
  int bits_ = 0;
};

// An ALAC frame holding PCM as it is (the "escape" frame every decoder
// handles): a channel pair element with the escape flag, the samples as
// 16-bit big endian, then the end tag.
QByteArray AlacUncompressed(const QByteArray& pcm) {
  BitWriter w;
  w.Put(1, 3);       // ID_CPE: a channel pair
  w.Put(0, 4);       // element instance tag
  w.Put(0, 12);      // unused
  w.Put(0b0001, 4);  // not partial, no shift, escape (uncompressed)
  for (int i = 0; i + 1 < pcm.size(); i += 2) {
    w.Put(qFromLittleEndian<quint16>(pcm.constData() + i), 16);
  }
  w.Put(7, 3);  // ID_END
  return w.Finish();
}

}  // namespace

Session::Session(const Options& options, std::unique_ptr<AudioSource> source,
                 QObject* parent)
    : QObject(parent),
      options_(options),
      source_(std::move(source)),
      connection_(new RtspConnection(this)),
      timing_(new QUdpSocket(this)),
      control_(new QUdpSocket(this)),
      audio_(new QUdpSocket(this)),
      sync_timer_(new QTimer(this)),
      feedback_timer_(new QTimer(this)),
      audio_timer_(new QTimer(this)) {
  connection_->set_client_name(options_.name);
  if (!options_.device_id.isEmpty()) {
    connection_->set_dacp_id(options_.device_id);
  }
  // The PIN of AirPlay 2 pairing is the password of AirPlay 1.
  if (options_.has_password) connection_->set_password(options_.pin);
  connect(connection_, SIGNAL(Connected()), SLOT(Connected()));
  connect(connection_, SIGNAL(Failed(QString)), SLOT(Fail(QString)));
  connect(timing_, SIGNAL(readyRead()), SLOT(TimingReadyRead()));
  connect(control_, SIGNAL(readyRead()), SLOT(ControlReadyRead()));

  sync_timer_->setInterval(1000);
  connect(sync_timer_, SIGNAL(timeout()), SLOT(SendSync()));
  // AirPlay 2 needs POST /feedback to keep the session alive.
  feedback_timer_->setInterval(options_.version == Version::kAirPlay2 ? 2000
                                                                      : 25000);
  connect(feedback_timer_, SIGNAL(timeout()), SLOT(SendFeedback()));
  audio_timer_->setTimerType(Qt::PreciseTimer);
  audio_timer_->setInterval(5);
  connect(audio_timer_, SIGNAL(timeout()), SLOT(SendAudio()));
}

Session::~Session() {}

void Session::Start(const QHostAddress& host, quint16 port) {
  host_ = host;
  port_ = port;
  connection_->Connect(host, port);
}

void Session::Fail(const QString& error) {
  if (done_) return;
  done_ = true;
  sync_timer_->stop();
  feedback_timer_->stop();
  audio_timer_->stop();
  emit Failed(error);
}

bool Session::Check(const RtspConnection::Response& response,
                    const QString& what) {
  if (response.code == 200) return true;
  if (response.code == 401) {
    Fail(options_.has_password
             ? what + " refused: wrong password"
             : what +
                   " refused: the receiver requires a password; pass "
                   "--pin or set AIRPLAY_PIN");
    return false;
  }
  Fail(QString("%1 failed: %2 %3")
           .arg(what)
           .arg(response.code)
           .arg(QString::fromUtf8(response.body)));
  return false;
}

// What a pairing step's error TLV means (HAP's kTLVError values).
QString Session::PairingError(int code, const tlv8::Items& items) const {
  const QByteArray error = tlv8::Get(items, tlv8::kError);
  const int value = error.isEmpty() ? -1 : quint8(error[0]);
  switch (value) {
    case 0x02:  // kTLVError_Authentication: the SRP proof didn't match
      return options_.has_password
                 ? "wrong password"
                 : "the receiver requires a password; pass --pin or set "
                   "AIRPLAY_PIN";
    case 0x03: {  // kTLVError_Backoff
      const QByteArray delay = tlv8::Get(items, tlv8::kRetryDelay);
      return QString("too many attempts; the receiver says to retry in %1 s")
          .arg(delay.isEmpty() ? QString("a while")
                               : QString::number(quint8(delay[0])));
    }
    case 0x05:  // kTLVError_MaxTries
      return "too many failed attempts";
    case 0x06:  // kTLVError_Unavailable
      return "the receiver isn't accepting pairing";
    case 0x07:  // kTLVError_Busy
      return "the receiver is busy pairing with another sender";
    default:
      return QString("refused (HTTP %1, error %2)")
          .arg(code)
          .arg(QString(error.toHex()));
  }
}

Headers Session::SessionHeaders() const {
  if (rtsp_session_.isEmpty()) return {};
  return {{"Session", rtsp_session_.toUtf8()}};
}

void Session::Connected() {
  const QHostAddress local = connection_->local_address();
  qCInfo(airplay_log).noquote()
      << QString("Connected to %1:%2 from %3, using %4")
             .arg(host_.toString())
             .arg(port_)
             .arg(local.toString(), VersionName(options_.version));

  // Receivers ask for the time while they set up, and may not answer SETUP
  // until they've had an answer, so this listens from the start.
  if (!timing_->bind(local, 0) || !control_->bind(local, 0) ||
      !audio_->bind(local, 0)) {
    Fail("couldn't listen on UDP ports");
    return;
  }
  GetInfo();
}

void Session::GetInfo() {
  connection_->Request(
      "GET", "/info", {}, QByteArray(), QByteArray(), "RTSP/1.0",
      [this](const RtspConnection::Response& r) {
        QVariant info;
        if (r.code == 200 && bplist::Read(r.body, &info)) {
          const QVariantMap map = info.toMap();
          qCInfo(airplay_log).noquote()
              << QString("Device: %1 (%2), source version %3")
                     .arg(map.value("name").toString(),
                          map.value("model").toString(),
                          map.value("sourceVersion").toString());
          if (map.value("statusFlags").toUInt() & kStatusPasswordRequired &&
              !options_.has_password) {
            Fail(map.value("name").toString() +
                 " requires a password: pass --pin or set AIRPLAY_PIN");
            return;
          }
        }
        if (options_.version == Version::kAirPlay1) {
          SetupAirPlay1();
        } else {
          Pair();
        }
      });
}

void Session::SetupAirPlay1() {
  const QString sdp =
      QString(
          "v=0\r\n"
          "o=iTunes %1 0 IN IP4 %2\r\n"
          "s=iTunes\r\n"
          "c=IN IP4 %3\r\n"
          "t=0 0\r\n"
          "m=audio 0 RTP/AVP 96\r\n"
          "a=rtpmap:96 AppleLossless\r\n"
          "a=fmtp:96 %4 0 16 40 10 14 2 255 0 0 44100\r\n")
          .arg(connection_->session_id())
          .arg(connection_->local_address().toString(), host_.toString())
          .arg(kFramesPerPacket);
  connection_->Rtsp(
      "ANNOUNCE", {}, "application/sdp", sdp.toUtf8(),
      [this](const RtspConnection::Response& r) {
        // A Mac that requires a password refuses AirPlay 1 until the sender
        // has paired as AirPlay 2 senders do, rather than asking for a
        // digest; then it takes ANNOUNCE on the encrypted connection.
        if (r.code == 403 && !paired_) {
          qCInfo(airplay_log) << "ANNOUNCE refused; pairing first";
          Pair();
          return;
        }
        if (!Check(r, "ANNOUNCE")) return;
        const QByteArray transport =
            QString(
                "RTP/AVP/UDP;unicast;interleaved=0-1;mode=record;"
                "control_port=%1;timing_port=%2")
                .arg(control_->localPort())
                .arg(timing_->localPort())
                .toUtf8();
        connection_->Rtsp(
            "SETUP", {{"Transport", transport}}, QByteArray(), QByteArray(),
            [this](const RtspConnection::Response& r) {
              if (!Check(r, "SETUP")) return;
              quint16 data_port = 0;
              quint16 control_port = 0;
              for (const QByteArray& option :
                   r.Header("Transport").split(';')) {
                const QList<QByteArray> kv = option.split('=');
                if (kv.size() != 2) continue;
                if (kv[0] == "server_port") data_port = kv[1].toUShort();
                if (kv[0] == "control_port") control_port = kv[1].toUShort();
              }
              if (!data_port || !control_port) {
                Fail("SETUP didn't give the receiver's ports");
                return;
              }
              rtsp_session_ = QString::fromUtf8(r.Header("Session"));
              if (rtsp_session_.isEmpty()) rtsp_session_ = "1";
              StartStreaming(data_port, control_port);
            });
      },
      kApprovalTimeoutMsec);
}

// HomeKit transient pairing: pair-setup M1 to M4 with a fixed or user PIN.
void Session::Pair() {
  const Headers headers = {{"X-Apple-HKP", "4"}, {"Connection", "keep-alive"}};
  connection_->Post(
      "/pair-pin-start", headers, QByteArray(),
      [](const RtspConnection::Response& r) {
        qCDebug(airplay_log) << "pair-pin-start:" << r.code;
      },
      kApprovalTimeoutMsec);

  const QByteArray m1 = tlv8::Write({
      {tlv8::kMethod, QByteArray(1, '\0')},
      {tlv8::kSeqNo, QByteArray(1, 1)},
      {tlv8::kFlags, QByteArray(1, char(tlv8::kFlagTransient))},
  });
  connection_->Post(
      "/pair-setup", headers, m1, [this](const RtspConnection::Response& r) {
        if (r.code != 200) {
          Fail(QString("pair-setup M1 refused: %1 (is \"Allow AirPlay for\" "
                       "set to anyone on the network?)")
                   .arg(r.code));
          return;
        }
        PairM3(r.body);
      });
}

void Session::PairM3(const QByteArray& response) {
  const tlv8::Items m2 = tlv8::Read(response);
  if (!tlv8::Get(m2, tlv8::kError).isNull()) {
    Fail("pair-setup M2: " + PairingError(200, m2));
    return;
  }
  const QByteArray salt = tlv8::Get(m2, tlv8::kSalt);
  const QByteArray b_public = tlv8::Get(m2, tlv8::kPublicKey);
  if (salt.isEmpty() || b_public.isEmpty()) {
    Fail("no salt or public key in pair-setup M2");
    return;
  }

  const SrpClient::Result srp =
      SrpClient().Process(options_.pin, salt, b_public);
  const QByteArray m3 = tlv8::Write({
      {tlv8::kSeqNo, QByteArray(1, 3)},
      {tlv8::kPublicKey, srp.public_key},
      {tlv8::kProof, srp.proof},
  });
  const QByteArray key = srp.key;
  connection_->Post(
      "/pair-setup", {{"X-Apple-HKP", "4"}, {"Connection", "keep-alive"}}, m3,
      [this, key](const RtspConnection::Response& r) {
        const tlv8::Items m4 = tlv8::Read(r.body);
        const QByteArray error = tlv8::Get(m4, tlv8::kError);
        if (r.code != 200 || !error.isNull()) {
          Fail("pair-setup M4: " + PairingError(r.code, m4));
          return;
        }
        shared_key_ = key;
        connection_->EnableEncryption(
            HkdfSha512("Control-Salt", "Control-Write-Encryption-Key", key),
            HkdfSha512("Control-Salt", "Control-Read-Encryption-Key", key));
        qCInfo(airplay_log) << "Paired (transient), connection encrypted";
        paired_ = true;
        if (options_.version == Version::kAirPlay1) {
          SetupAirPlay1();
        } else {
          SetupAirPlay2Session();
        }
      });
}

void Session::SetupAirPlay2Session() {
  QVariantMap body;
  body["deviceID"] = QString(connection_->dacp_id());
  body["macAddress"] = "AA:BB:CC:DD:EE:FF";
  body["sessionUUID"] =
      QUuid::createUuid().toString(QUuid::WithoutBraces).toUpper();
  body["timingPort"] = timing_->localPort();
  body["timingProtocol"] = "NTP";
  body["isMultiSelectAirPlay"] = true;
  body["groupContainsGroupLeader"] = false;
  body["model"] = "Clementine";
  body["name"] = options_.name;
  body["osBuildVersion"] = "20F66";
  body["osName"] = "iPhone OS";
  body["osVersion"] = "16.5";
  body["senderSupportsRelay"] = false;
  body["sourceVersion"] = "690.7.1";
  body["statsCollectionEnabled"] = false;

  connection_->Rtsp(
      "SETUP", {}, bplist::kContentType, bplist::Write(body),
      [this](const RtspConnection::Response& r) {
        if (!Check(r, "SETUP")) return;
        QVariant reply;
        bplist::Read(r.body, &reply);
        event_port_ = reply.toMap().value("eventPort").toUInt();
        qCDebug(airplay_log) << "Event port" << event_port_;
        if (event_port_) {
          event_cipher_.reset(new FrameCipher(
              HkdfSha512("Events-Salt", "Events-Read-Encryption-Key",
                         shared_key_),
              HkdfSha512("Events-Salt", "Events-Write-Encryption-Key",
                         shared_key_)));
          event_socket_ = new QTcpSocket(this);
          connect(event_socket_, SIGNAL(connected()), SLOT(EventConnected()));
          connect(event_socket_,
                  SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
                  SLOT(EventError()));
          connect(event_socket_, SIGNAL(readyRead()), SLOT(EventReadyRead()));
          ConnectEventChannel();
        } else {
          SetupAirPlay2Stream();
        }
      });
}

void Session::ConnectEventChannel() {
  ++event_attempts_;
  event_socket_->connectToHost(host_, event_port_);
}

void Session::EventConnected() { SetupAirPlay2Stream(); }

void Session::EventError() {
  if (event_socket_->state() == QAbstractSocket::ConnectedState || done_) {
    qCDebug(airplay_log) << "Event channel:" << event_socket_->errorString();
    return;
  }
  if (event_attempts_ >= kEventAttempts) {
    Fail("couldn't open the event channel: " + event_socket_->errorString());
    return;
  }
  qCDebug(airplay_log) << "Event channel:" << event_socket_->errorString()
                       << "- retrying";
  QTimer::singleShot(1000, this, SLOT(ConnectEventChannel()));
}

// The receiver sends RTSP requests on the event channel; each gets 200 OK, as
// Apple's senders do.
void Session::EventReadyRead() {
  if (!event_cipher_->Decrypt(event_socket_->readAll(), &event_buffer_)) {
    qCWarning(airplay_log) << "Couldn't decrypt the event channel";
    event_socket_->abort();
    return;
  }
  while (true) {
    const int end = event_buffer_.indexOf("\r\n\r\n");
    if (end < 0) return;
    int length = 0;
    QByteArray cseq;
    const QList<QByteArray> lines = event_buffer_.left(end).split('\n');
    for (const QByteArray& line : lines) {
      const int colon = line.indexOf(':');
      if (colon < 0) continue;
      const QByteArray key = line.left(colon).trimmed().toLower();
      if (key == "content-length")
        length = line.mid(colon + 1).trimmed().toInt();
      if (key == "cseq") cseq = line.mid(colon + 1).trimmed();
    }
    if (event_buffer_.size() < end + 4 + length) return;
    event_buffer_.remove(0, end + 4 + length);
    qCDebug(airplay_log) << "Event:" << lines.value(0).trimmed();
    event_socket_->write(event_cipher_->Encrypt(
        "RTSP/1.0 200 OK\r\nContent-Length: 0\r\nAudio-Latency: 0\r\nCSeq: " +
        cseq + "\r\n\r\n"));
  }
}

void Session::SetupAirPlay2Stream() {
  // The receiver takes whatever key we send. It's random rather than derived
  // from the pairing like the others, which pyatv does: its derivation gives
  // the key the receiver encrypts the event channel with, and both count
  // nonces from 0, so the two would reuse key and nonce together.
  audio_key_.resize(32);
  QRandomGenerator::system()->fillRange(
      reinterpret_cast<quint32*>(audio_key_.data()),
      audio_key_.size() / sizeof(quint32));
  QVariantMap stream;
  stream["audioFormat"] = 0x800;  // L16, 44.1 kHz, stereo
  stream["audioMode"] = "default";
  stream["controlPort"] = control_->localPort();
  stream["ct"] = 1;
  stream["isMedia"] = true;
  stream["latencyMax"] = 88200;
  stream["latencyMin"] = 11025;
  stream["shk"] = audio_key_;
  stream["spf"] = kFramesPerPacket;
  stream["sr"] = kSampleRate;
  stream["type"] = 0x60;  // Realtime audio
  stream["supportsDynamicStreamID"] = false;
  stream["streamConnectionID"] = qint64(connection_->session_id());
  QVariantMap body;
  body["streams"] = QVariantList{stream};

  connection_->Rtsp(
      "SETUP", {}, bplist::kContentType, bplist::Write(body),
      [this](const RtspConnection::Response& r) {
        if (!Check(r, "SETUP")) return;
        QVariant reply;
        bplist::Read(r.body, &reply);
        const QVariantMap s =
            reply.toMap().value("streams").toList().value(0).toMap();
        const quint16 data_port = s.value("dataPort").toUInt();
        const quint16 control_port = s.value("controlPort").toUInt();
        if (!data_port || !control_port) {
          Fail("SETUP didn't give the receiver's ports");
          return;
        }
        StartStreaming(data_port, control_port);
      });
}

void Session::StartStreaming(quint16 data_port, quint16 control_port) {
  qCInfo(airplay_log).noquote()
      << QString("Receiver ports: audio %1, control %2")
             .arg(data_port)
             .arg(control_port);
  data_port_ = data_port;
  remote_control_port_ = control_port;
  start_ts_ = NtpToTimestamp(NtpNow());
  head_ts_ = start_ts_;
  seq_ = quint16(QRandomGenerator::global()->generate());

  SendSync();
  sync_timer_->start();

  connection_->Rtsp(
      "RECORD", SessionHeaders(), QByteArray(), QByteArray(),
      [this](const RtspConnection::Response& r) {
        if (!Check(r, "RECORD")) return;
        Headers headers = SessionHeaders();
        headers.append(qMakePair(QByteArray("Range"), QByteArray("npt=0-")));
        headers.append(qMakePair(
            QByteArray("RTP-Info"),
            QString("seq=%1;rtptime=%2").arg(seq_).arg(RtpTime()).toUtf8()));
        connection_->Rtsp("FLUSH", headers, QByteArray(), QByteArray(),
                          [this](const RtspConnection::Response& r) {
                            qCDebug(airplay_log) << "FLUSH:" << r.code;
                            if (options_.volume >= 0) {
                              SetVolume([this]() { StartAudio(); });
                            } else {
                              StartAudio();
                            }
                          });
      });
}

void Session::StartAudio() {
  if (done_) return;
  qCInfo(airplay_log) << "Streaming";
  feedback_timer_->start();
  started_.start();
  last_report_.start();
  audio_timer_->start();
  SendAudio();
}

// Volume from 0 to 100, which AirPlay takes as -30 to 0 dB.
void Session::SetVolume(std::function<void()> done) {
  const double db = options_.volume <= 0
                        ? -144.0
                        : -30.0 + 30.0 * qMin(options_.volume, 100) / 100.0;
  connection_->Rtsp("SET_PARAMETER", SessionHeaders(), "text/parameters",
                    QString("volume: %1\r\n").arg(db, 0, 'f', 6).toUtf8(),
                    [done](const RtspConnection::Response& r) {
                      if (r.code != 200) {
                        qCWarning(airplay_log)
                            << "Setting the volume failed:" << r.code;
                      }
                      done();
                    });
}

quint32 Session::RtpTime() const {
  return quint32(head_ts_ - start_ts_ + kLatency);
}

// Sends whatever packets are due, pacing them to real time.
void Session::SendAudio() {
  const qint64 elapsed = started_.nsecsElapsed();
  while (!done_ && sent_frames_ * 1000000000LL / kSampleRate <= elapsed) {
    SendPacket();
  }
}

void Session::SendPacket() {
  // Once the source ends, keep sending silence for the latency so what was
  // sent gets played out.
  if (padding_ >= kLatency) {
    Finish();
    return;
  }

  QByteArray pcm(kFramesPerPacket * kFrameSize, '\0');
  const int n = source_->Read(pcm.data(), pcm.size());
  if (n <= 0) {
    padding_ += kFramesPerPacket;
  } else if (n < pcm.size()) {
    pcm.replace(n, pcm.size() - n, QByteArray(pcm.size() - n, '\0'));
  }

  QByteArray payload;
  if (options_.version == Version::kAirPlay1) {
    // AirPlay 1 receivers all take ALAC, so the PCM goes in uncompressed
    // ALAC frames.
    payload = AlacUncompressed(pcm);
  } else {
    // AirPlay 2 takes L16, which is big endian.
    payload.resize(pcm.size());
    for (int i = 0; i + 1 < pcm.size(); i += 2) {
      payload[i] = pcm[i + 1];
      payload[i + 1] = pcm[i];
    }
  }

  QByteArray packet;
  packet.append(char(0x80));
  packet.append(char(first_packet_ ? 0xE0 : 0x60));
  packet += Bytes(qToBigEndian(seq_));
  packet += Bytes(qToBigEndian(RtpTime()));
  packet += Bytes(qToBigEndian(connection_->session_id()));
  if (!audio_key_.isEmpty()) {
    const QByteArray nonce = Nonce(audio_nonce_++);
    packet += Seal(audio_key_, nonce, packet.mid(4, 8), payload);
    packet += nonce.mid(4);
  } else {
    packet += payload;
  }
  first_packet_ = false;

  audio_->writeDatagram(packet, host_, data_port_);
  backlog_.insert(seq_, packet);
  backlog_.remove(quint16(seq_ - kBacklog));
  ++seq_;
  head_ts_ += kFramesPerPacket;
  sent_frames_ += kFramesPerPacket;

  if (last_report_.elapsed() >= 5000) {
    qCInfo(airplay_log).noquote()
        << QString("Sent %1 s of audio")
               .arg(double(sent_frames_) / kSampleRate, 0, 'f', 1);
    last_report_.restart();
  }
}

void Session::Finish() {
  done_ = true;
  audio_timer_->stop();
  sync_timer_->stop();
  feedback_timer_->stop();
  qCInfo(airplay_log).noquote()
      << QString("Finished after %1 s")
             .arg(started_.elapsed() / 1000.0, 0, 'f', 1);
  connection_->Rtsp(
      "TEARDOWN", SessionHeaders(), QByteArray(), QByteArray(),
      [this](const RtspConnection::Response&) { emit Finished(); });
}

// A sync packet: the RTP time being played now, and the NTP time and RTP
// time of the next packet.
void Session::SendSync() {
  const quint32 rtptime = RtpTime();
  QByteArray p;
  p.append(char(first_sync_ ? 0x90 : 0x80));
  p.append(char(0xD4));
  p += Bytes(qToBigEndian(quint16(7)));
  p += Bytes(qToBigEndian(quint32(rtptime - kLatency)));
  p += Bytes(qToBigEndian(TimestampToNtp(head_ts_)));
  p += Bytes(qToBigEndian(rtptime));
  control_->writeDatagram(p, host_, remote_control_port_);
  first_sync_ = false;
}

void Session::ControlReadyRead() {
  while (control_->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = control_->receiveDatagram();
    const QByteArray data = datagram.data();
    if (data.size() >= 8 && (quint8(data[1]) & 0x7F) == 0x55) {
      const quint16 lost = qFromBigEndian<quint16>(data.constData() + 4);
      const quint16 count = qFromBigEndian<quint16>(data.constData() + 6);
      qCDebug(airplay_log) << "Resend request for" << count << "packets from"
                           << lost;
      for (quint16 i = 0; i < count; ++i) {
        const QByteArray packet = backlog_.value(quint16(lost + i));
        if (packet.isEmpty()) continue;
        QByteArray resend;
        resend.append(char(0x80));
        resend.append(char(0xD6));
        resend += packet.mid(2, 2);
        resend += packet;
        control_->writeDatagram(resend, datagram.senderAddress(),
                                datagram.senderPort());
      }
    } else if (data.size() >= 2) {
      qCDebug(airplay_log) << "Control packet type" << Qt::hex
                           << (quint8(data[1]) & 0x7F);
    }
  }
}

// Answers the receiver's NTP-style timing requests.
void Session::TimingReadyRead() {
  while (timing_->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = timing_->receiveDatagram();
    const QByteArray data = datagram.data();
    qCDebug(airplay_log).noquote()
        << QString("Timing request from %1:%2")
               .arg(datagram.senderAddress().toString())
               .arg(datagram.senderPort());
    if (data.size() < 32) continue;
    const quint64 now = NtpNow();
    QByteArray response;
    response.append(data[0]);
    response.append(char(0x53 | 0x80));
    response += Bytes(qToBigEndian(quint16(7)));
    response += QByteArray(4, '\0');
    // Reference time: when they sent the request.
    response += data.mid(24, 8);
    response += Bytes(qToBigEndian(now));
    response += Bytes(qToBigEndian(now));
    timing_->writeDatagram(response, datagram.senderAddress(),
                           datagram.senderPort());
  }
}

void Session::SendFeedback() {
  connection_->Request("POST", "/feedback", {}, QByteArray(), QByteArray(),
                       "RTSP/1.0", [](const RtspConnection::Response& r) {
                         if (r.code != 200) {
                           qCDebug(airplay_log) << "feedback:" << r.code;
                         }
                       });
}

}  // namespace airplay
