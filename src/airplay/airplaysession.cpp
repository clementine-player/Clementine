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

#include "airplaysession.h"

#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QSettings>
#include <QStringList>
#include <QSysInfo>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUuid>
#include <QVariantMap>
#include <QtEndian>
#include <chrono>
#include <cmath>

#include "airplaycrypto.h"
#include "airplayplist.h"
#include "airplayrtspconnection.h"
#include "airplaysrp.h"
#include "airplaystream.h"
#include "core/logging.h"

namespace {

// The PIN of transient pairing, which receivers that don't need a password
// take, as pyatv and Apple's senders use. A Mac shows it as its PIN when
// asked to with /pair-pin-start.
const char kTransientPin[] = "3939";

const char kPlistType[] = "application/x-apple-binary-plist";
const char kSettingsGroup[] = "AirPlay";
// How often to try connecting to the event channel, a second apart: a
// receiver may not be listening on it straight away, as pyatv found.
const int kEventAttempts = 5;

// How long the first SETUP takes before it's probably waiting for the
// receiver's user.
const int kApprovalNoticeMsec = 1000;

}  // namespace

AirPlaySession::Options::Options()
    : name(DefaultName()),
      show_pin(false),
      force_airplay1(false),
      connect_timeout_msec(AirPlayRtspConnection::kConnectTimeoutMsec),
      request_timeout_msec(AirPlayRtspConnection::kTimeoutMsec),
      // A Mac holds the first SETUP until its user answers, which can take
      // longer than other responses may. A Mac gives up after 15 s and
      // answers 400; others may wait longer.
      approval_timeout_msec(30000) {}

AirPlaySession::AirPlaySession(const Options& options, QObject* parent)
    : QObject(parent),
      options_(options),
      airplay1_(false),
      connection_(new AirPlayRtspConnection(this)),
      state_(State::Closed),
      timing_(new QUdpSocket(this)),
      approval_timer_(new QTimer(this)),
      control_(new QUdpSocket(this)),
      sync_timer_(new QTimer(this)),
      feedback_timer_(new QTimer(this)),
      event_socket_(new QTcpSocket(this)),
      event_port_(0),
      event_attempts_(0),
      stream_(nullptr),
      volume_(-1) {
  connection_->set_client_name(options_.name);
  connection_->set_timeouts(options_.connect_timeout_msec,
                            options_.request_timeout_msec);
  connect(connection_, SIGNAL(Connected()), SLOT(Connected()));
  connect(connection_, SIGNAL(Failed(QString)), SLOT(Fail(QString)));
  connect(timing_, SIGNAL(readyRead()), SLOT(TimingReadyRead()));
  approval_timer_->setSingleShot(true);
  approval_timer_->setInterval(kApprovalNoticeMsec);
  connect(approval_timer_, SIGNAL(timeout()), SIGNAL(WaitingForApproval()));

  // As often as Apple's senders send them.
  sync_timer_->setInterval(1000);
  connect(sync_timer_, SIGNAL(timeout()), SLOT(SendSync()));
  // A receiver ends a session it doesn't hear from, as pyatv found.
  feedback_timer_->setInterval(2000);
  connect(feedback_timer_, SIGNAL(timeout()), SLOT(SendFeedback()));

  connect(event_socket_, SIGNAL(connected()), SLOT(EventConnected()));
  connect(event_socket_, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
          SLOT(EventError()));
  connect(event_socket_, SIGNAL(readyRead()), SLOT(EventReadyRead()));
}

// Tears the session down even when Clementine quits: a Mac asks its user
// again about a sender that left a session unfinished.
AirPlaySession::~AirPlaySession() { Close(); }

QString AirPlaySession::SavedSenderId() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString id = s.value("sender_id").toString();
  if (id.isEmpty()) {
    QStringList bytes;
    for (int i = 0; i < 6; ++i) {
      quint8 byte = quint8(QRandomGenerator::system()->generate());
      // A locally administered, unicast address, as made-up ones are.
      if (i == 0) byte = (byte & 0xFC) | 0x02;
      bytes << QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
    }
    id = bytes.join(':');
    s.setValue("sender_id", id);
  }
  return id;
}

QByteArray AirPlaySession::SavedIdentitySeed() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QByteArray seed = QByteArray::fromHex(s.value("identity_seed").toByteArray());
  if (seed.size() != 32) {
    seed = airplay::RandomKey();
    s.setValue("identity_seed", seed.toHex());
  }
  return seed;
}

QString AirPlaySession::DefaultName() {
  return tr("Clementine on %1").arg(QSysInfo::machineHostName());
}

void AirPlaySession::Open(const AirPlayDevice& device) {
  Close();
  device_ = device;
  airplay1_ = device.airplay1 || options_.force_airplay1;
  state_ = State::Connecting;
  if (airplay1_ && !device.plain_audio) {
    Fail(tr("%1 only takes encrypted AirPlay 1 audio, which Clementine "
            "can't send")
             .arg(device.name));
    return;
  }
  qLog(Info) << "Connecting to AirPlay receiver" << device
             << (airplay1_ ? "with AirPlay 1" : "");
  connection_->set_password(options_.password);
  connection_->Connect(device.address, device.port);
}

void AirPlaySession::Close() {
  if (state_ == State::SetUp || state_ == State::Ready) {
    // Sent, but not waited for.
    connection_->Rtsp("TEARDOWN", {}, QByteArray(), QByteArray(), nullptr);
  }
  state_ = State::Closed;
  approval_timer_->stop();
  sync_timer_->stop();
  feedback_timer_->stop();
  connection_->Close();
  event_socket_->abort();
  event_cipher_.reset();
  event_buffer_.clear();
  timing_->close();
  control_->close();
  if (stream_) {
    emit StreamEnding();
    delete stream_;
    stream_ = nullptr;
  }
}

void AirPlaySession::Fail(const QString& error) {
  if (state_ == State::Closed) return;
  qLog(Warning) << "AirPlay receiver" << device_.name << "failed:" << error;
  Close();
  emit Failed(error);
}

void AirPlaySession::Connected() {
  state_ = State::Pairing;
  // Receivers ask for the time while they set up, and may not answer SETUP
  // until they've had an answer, so this listens from the start.
  const QHostAddress local = connection_->local_address();
  if (!timing_->bind(local, 0) || !control_->bind(local, 0)) {
    Fail(tr("Couldn't listen for the AirPlay receiver"));
    return;
  }
  if (!airplay1_) {
    GetInfo();
    return;
  }
  // AirPlay 1 receivers say whether they want a password as they're
  // advertised, and may not have GET /info.
  if (device_.password_required() && options_.password.isEmpty() &&
      !persistent()) {
    qLog(Info) << "AirPlay receiver" << device_.name << "wants a password";
    Close();
    emit PasswordNeeded(false);
    return;
  }
  state_ = State::SettingUp;
  Announce();
}

void AirPlaySession::GetInfo() {
  connection_->Request(
      "GET", "/info", {}, QByteArray(), QByteArray(), "RTSP/1.0",
      [this](const AirPlayRtspConnection::Response& r) {
        QVariant info;
        if (r.code == 200 && airplay::bplist::Read(r.body, &info)) {
          const QVariantMap map = info.toMap();
          qLog(Debug) << "AirPlay receiver" << map.value("name").toString()
                      << map.value("model").toString() << "source version"
                      << map.value("sourceVersion").toString();
          if (map.value("statusFlags").toUInt() &
                  AirPlayDevice::PasswordRequired &&
              options_.password.isEmpty() && !persistent()) {
            qLog(Info) << "AirPlay receiver" << device_.name
                       << "wants a password";
            Close();
            emit PasswordNeeded(false);
            return;
          }
        }
        Pair();
      });
}

// HomeKit transient pairing: pair-setup M1 to M4 with the fixed PIN.
QString AirPlaySession::Pin() const {
  return options_.password.isEmpty() ? kTransientPin : options_.password;
}

QByteArray AirPlaySession::PairingId() const {
  return (options_.sender_id.isEmpty() ? QString(connection_->dacp_id())
                                       : options_.sender_id)
      .toUtf8();
}

// X-Apple-HKP says which kind of HomeKit pairing: 4 for transient, 3 for
// pairing that's remembered, as pyatv sends them.
AirPlayRtspConnection::Headers AirPlaySession::Headers() const {
  return {{"X-Apple-HKP", persistent() ? "3" : "4"},
          {"Connection", "keep-alive"}};
}

void AirPlaySession::Pair() {
  if (persistent() && !options_.receiver_public_key.isEmpty()) {
    PairVerify();
    return;
  }
  if (options_.show_pin) {
    // The receiver shows a PIN, which its user tells Clementine.
    connection_->Post(
        "/pair-pin-start", Headers(), QByteArray(),
        [this](const AirPlayRtspConnection::Response& r) {
          if (r.code != 200) {
            Fail(tr("%1 didn't show a PIN to pair with (HTTP %2)")
                     .arg(device_.name)
                     .arg(r.code));
            return;
          }
          emit PinShown();
        },
        options_.approval_timeout_msec);
    return;
  }
  PairSetup();
}

void AirPlaySession::SubmitPin(const QString& pin) {
  if (state_ != State::Pairing) return;
  options_.password = pin;
  PairSetup();
}

// HomeKit pair-setup M1 to M4, with the fixed PIN, a password or a PIN the
// receiver showed: transient unless this sender has long-term keys.
void AirPlaySession::PairSetup() {
  namespace tlv8 = airplay::tlv8;
  tlv8::Items m1 = {
      {tlv8::kMethod, QByteArray(1, '\0')},
      {tlv8::kSeqNo, QByteArray(1, 1)},
  };
  if (!persistent()) {
    m1.append({tlv8::kFlags, QByteArray(1, char(tlv8::kFlagTransient))});
  }
  connection_->Post("/pair-setup", Headers(), tlv8::Write(m1),
                    [this](const AirPlayRtspConnection::Response& r) {
                      if (r.code != 200) {
                        Fail(tr("%1 refused to pair (HTTP %2). Is it set to "
                                "allow AirPlay from anyone on the network?")
                                 .arg(device_.name)
                                 .arg(r.code));
                        return;
                      }
                      PairM3(r.body);
                    });
}

void AirPlaySession::PairM3(const QByteArray& response) {
  namespace tlv8 = airplay::tlv8;
  const tlv8::Items m2 = tlv8::Read(response);
  if (!tlv8::Get(m2, tlv8::kError).isNull()) {
    Fail(PairingError(200, m2));
    return;
  }
  const QByteArray salt = tlv8::Get(m2, tlv8::kSalt);
  const QByteArray b_public = tlv8::Get(m2, tlv8::kPublicKey);
  if (salt.isEmpty() || b_public.isEmpty()) {
    Fail(tr("%1 didn't pair as AirPlay receivers do").arg(device_.name));
    return;
  }

  const airplay::SrpClient::Result srp =
      airplay::SrpClient().Process(Pin(), salt, b_public);
  const QByteArray m3 = tlv8::Write({
      {tlv8::kSeqNo, QByteArray(1, 3)},
      {tlv8::kPublicKey, srp.public_key},
      {tlv8::kProof, srp.proof},
  });
  const QByteArray key = srp.key;
  connection_->Post(
      "/pair-setup", Headers(), m3,
      [this, key](const AirPlayRtspConnection::Response& r) {
        const tlv8::Items m4 = tlv8::Read(r.body);
        if (r.code == 200 && tlv8::Get(m4, tlv8::kError) == QByteArray(1, 2)) {
          // The PIN or password didn't match.
          qLog(Info) << "AirPlay receiver" << device_.name
                     << "refused the password or PIN";
          const bool wrong = !options_.password.isEmpty();
          Close();
          emit PasswordNeeded(wrong);
          return;
        }
        if (r.code != 200 || !tlv8::Get(m4, tlv8::kError).isNull()) {
          Fail(PairingError(r.code, m4));
          return;
        }
        if (persistent()) {
          PairM5(key);
        } else {
          PairedWith(key);
        }
      });
}

void AirPlaySession::PairedWith(const QByteArray& shared) {
  // The keys of the encrypted connection, as HomeKit derives them.
  connection_->EnableEncryption(
      airplay::HkdfSha512("Control-Salt", "Control-Write-Encryption-Key",
                          shared),
      airplay::HkdfSha512("Control-Salt", "Control-Read-Encryption-Key",
                          shared));
  qLog(Info) << "Paired with AirPlay receiver" << device_.name;
  shared_key_ = shared;
  state_ = State::SettingUp;
  emit Paired();
  if (airplay1_) {
    Announce();
  } else {
    SetupSession();
  }
}

QString AirPlaySession::PairingError(int code,
                                     const airplay::tlv8::Items& items) const {
  namespace tlv8 = airplay::tlv8;
  const QByteArray error = tlv8::Get(items, tlv8::kError);
  // HAP's kTLVError values.
  switch (error.isEmpty() ? -1 : quint8(error[0])) {
    case 0x02:  // Authentication: the SRP proof didn't match.
      return tr("%1 refused to pair").arg(device_.name);
    case 0x03: {  // Backoff
      const QByteArray delay = tlv8::Get(items, tlv8::kRetryDelay);
      return tr("%1 refused to pair after too many attempts. Try again in %2 "
                "seconds.")
          .arg(device_.name)
          // A guess, when it doesn't say.
          .arg(delay.isEmpty() ? 60 : quint8(delay[0]));
    }
    case 0x05:  // MaxTries
      return tr("%1 refused to pair after too many attempts").arg(device_.name);
    case 0x06:  // Unavailable
      return tr("%1 isn't accepting new AirPlay senders").arg(device_.name);
    case 0x07:  // Busy
      return tr("%1 is busy pairing with another AirPlay sender")
          .arg(device_.name);
    default:
      return tr("%1 refused to pair (HTTP %2, error %3)")
          .arg(device_.name)
          .arg(code)
          .arg(QString(error.toHex()));
  }
}

// The first SETUP: who this is, and where to ask for the time. The keys are
// those Apple's senders send, as pyatv sends them; the timing protocol is
// NTP rather than PTP, which AirPlay 2 receivers also take.
void AirPlaySession::SetupSession() {
  QVariantMap body;
  const QString sender_id = options_.sender_id.isEmpty()
                                ? QString(connection_->dacp_id())
                                : options_.sender_id;
  body["deviceID"] = sender_id;
  body["macAddress"] = sender_id;
  body["sessionUUID"] =
      QUuid::createUuid().toString(QUuid::WithoutBraces).toUpper();
  body["timingPort"] = timing_->localPort();
  body["timingProtocol"] = "NTP";
  body["isMultiSelectAirPlay"] = true;
  body["groupContainsGroupLeader"] = false;
  body["model"] = "Clementine";
  body["name"] = options_.name;
  body["senderSupportsRelay"] = false;
  body["statsCollectionEnabled"] = false;
  approval_timer_->start();
  connection_->Rtsp(
      "SETUP", {}, kPlistType, airplay::bplist::Write(body),
      [this](const AirPlayRtspConnection::Response& r) {
        if (!CheckAllowed(r.code)) return;
        if (r.code != 200) {
          Fail(tr("%1 refused to set up a session (HTTP %2)")
                   .arg(device_.name)
                   .arg(r.code));
          return;
        }
        qLog(Info) << "Set up a session with AirPlay receiver" << device_.name;
        state_ = State::SetUp;
        emit SetUp();

        QVariant reply;
        airplay::bplist::Read(r.body, &reply);
        event_port_ = reply.toMap().value("eventPort").toUInt();
        if (!event_port_) {
          SetupStream();
          return;
        }
        // The receiver writes with the Events-Write key.
        event_cipher_.reset(new airplay::FrameCipher(
            airplay::HkdfSha512("Events-Salt", "Events-Read-Encryption-Key",
                                shared_key_),
            airplay::HkdfSha512("Events-Salt", "Events-Write-Encryption-Key",
                                shared_key_)));
        event_attempts_ = 0;
        ConnectEventChannel();
      },
      options_.approval_timeout_msec);
}

// Answers the receiver's NTP-style requests for the time.
void AirPlaySession::TimingReadyRead() {
  while (timing_->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = timing_->receiveDatagram();
    const QByteArray data = datagram.data();
    if (data.size() < 32) continue;
    const quint64 now = AirPlayStream::NtpNow();
    // RAOP's timing packets: an RTP-like header with payload type 0x52 for
    // a request and 0x53 for the reply (with the marker bit), sequence
    // number 7, then three NTP timestamps, as pyatv and shairport-sync have
    // them.
    QByteArray response;
    response.append(data[0]);
    response.append(char(0x53 | 0x80));
    response += airplay::Bytes(qToBigEndian(quint16(7)));
    response += QByteArray(4, '\0');
    // Reference time: when they sent the request.
    response += data.mid(24, 8);
    response += airplay::Bytes(qToBigEndian(now));
    response += airplay::Bytes(qToBigEndian(now));
    timing_->writeDatagram(response, datagram.senderAddress(),
                           datagram.senderPort());
  }
}

bool AirPlaySession::CheckAllowed(int code) {
  const bool asked = !approval_timer_->isActive();
  approval_timer_->stop();
  if (code != 400 || !asked) return true;
  qLog(Info) << "AirPlay receiver" << device_.name
             << "didn't allow this sender";
  Close();
  emit NotAllowed(
      tr("%1 didn't allow %2 to play there").arg(device_.name, options_.name));
  return false;
}

bool AirPlaySession::Check(const QByteArray& what, int code) {
  if (code == 200) return true;
  Fail(tr("%1 refused %2 (HTTP %3)")
           .arg(device_.name, QString::fromUtf8(what))
           .arg(code));
  return false;
}

void AirPlaySession::ConnectEventChannel() {
  if (state_ != State::SetUp) return;
  ++event_attempts_;
  event_socket_->connectToHost(device_.address, event_port_);
}

void AirPlaySession::EventConnected() { SetupStream(); }

void AirPlaySession::EventError() {
  if (state_ != State::SetUp) return;
  if (event_attempts_ >= kEventAttempts) {
    Fail(tr("Couldn't connect to %1's event channel: %2")
             .arg(device_.name, event_socket_->errorString()));
    return;
  }
  qLog(Debug) << "AirPlay event channel:" << event_socket_->errorString();
  event_socket_->abort();
  QTimer::singleShot(1000, this, SLOT(ConnectEventChannel()));
}

// The receiver sends RTSP requests on the event channel; each gets 200 OK, as
// Apple's senders do.
void AirPlaySession::EventReadyRead() {
  if (!event_cipher_ ||
      !event_cipher_->Decrypt(event_socket_->readAll(), &event_buffer_)) {
    qLog(Warning) << "Couldn't decrypt AirPlay event channel";
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
      const QByteArray value = line.mid(colon + 1).trimmed();
      if (key == "content-length") length = value.toInt();
      if (key == "cseq") cseq = value;
    }
    if (event_buffer_.size() < end + 4 + length) return;
    event_buffer_.remove(0, end + 4 + length);
    qLog(Debug) << "AirPlay event:" << lines.value(0).trimmed();
    event_socket_->write(event_cipher_->Encrypt(
        QByteArray("RTSP/1.0 200 OK\r\nContent-Length: 0\r\n"
                   "Audio-Latency: 0\r\nCSeq: ") +
        cseq + "\r\n\r\n"));
  }
}

// The second SETUP: a stream of realtime L16 audio.
void AirPlaySession::SetupStream() {
  // The receiver takes whatever key it's sent. It's random rather than
  // derived from the pairing as the other keys are, as pyatv derives it:
  // that gives the key the receiver encrypts the event channel with, and
  // both count nonces from 0, so the two would reuse key and nonce together.
  audio_key_.resize(32);
  QRandomGenerator::system()->fillRange(
      reinterpret_cast<quint32*>(audio_key_.data()),
      audio_key_.size() / sizeof(quint32));

  QVariantMap stream;
  // The format and stream type pyatv uses: 0x800 is L16 at 44.1 kHz in
  // stereo, and type 0x60 (96) is realtime audio, rather than buffered (103).
  stream["audioFormat"] = 0x800;
  stream["audioMode"] = "default";
  stream["controlPort"] = control_->localPort();
  stream["ct"] = 1;  // PCM
  stream["isMedia"] = true;
  // 2 s and 0.25 s, as pyatv asks for.
  stream["latencyMax"] = 88200;
  stream["latencyMin"] = 11025;
  stream["shk"] = audio_key_;
  stream["spf"] = AirPlayStream::kFramesPerPacket;
  stream["sr"] = AirPlayStream::kSampleRate;
  stream["type"] = 0x60;  // Realtime audio
  stream["supportsDynamicStreamID"] = false;
  stream["streamConnectionID"] = qint64(connection_->session_id());
  QVariantMap body;
  body["streams"] = QVariantList{stream};
  connection_->Rtsp(
      "SETUP", {}, kPlistType, airplay::bplist::Write(body),
      [this](const AirPlayRtspConnection::Response& r) {
        if (!Check("SETUP", r.code)) return;
        QVariant reply;
        airplay::bplist::Read(r.body, &reply);
        const QVariantMap s =
            reply.toMap().value("streams").toList().value(0).toMap();
        const quint16 data_port = s.value("dataPort").toUInt();
        const quint16 control_port = s.value("controlPort").toUInt();
        if (!data_port || !control_port) {
          Fail(tr("%1 didn't say where to send audio").arg(device_.name));
          return;
        }
        StartStream(data_port, control_port);
      });
}

void AirPlaySession::StartStream(quint16 data_port, quint16 control_port) {
  stream_ =
      new AirPlayStream(device_.address, data_port, control_port, control_,
                        audio_key_, connection_->session_id(), this);
  stream_->SendSync();
  sync_timer_->start();

  quint16 seq;
  quint32 rtptime;
  stream_->Next(&seq, &rtptime);
  const AirPlayRtspConnection::Headers start = {
      {"Range", "npt=0-"},
      {"RTP-Info",
       QString("seq=%1;rtptime=%2").arg(seq).arg(rtptime).toUtf8()}};
  // AirPlay 1 says where the stream starts with RECORD; AirPlay 2 with a
  // FLUSH after it.
  connection_->Rtsp(
      "RECORD", airplay1_ ? start : AirPlayRtspConnection::Headers(),
      QByteArray(), QByteArray(),
      [this, start](const AirPlayRtspConnection::Response& r) {
        if (!Check("RECORD", r.code)) return;
        if (airplay1_) {
          BecomeReady();
          return;
        }
        connection_->Rtsp("FLUSH", start, QByteArray(), QByteArray(),
                          [this](const AirPlayRtspConnection::Response& r) {
                            if (!Check("FLUSH", r.code)) return;
                            BecomeReady();
                          });
      });
}

void AirPlaySession::BecomeReady() {
  state_ = State::Ready;
  // AirPlay 1 receivers don't have /feedback.
  if (!airplay1_) feedback_timer_->start();
  if (volume_ >= 0) SetVolume(volume_);
  qLog(Info) << "Streaming to AirPlay receiver" << device_.name;
  emit Ready();
}

// AirPlay 1's ANNOUNCE: an SDP saying the audio is ALAC, unencrypted. The
// fmtp is ALAC's "magic cookie", as AirPort Express senders send it: frame
// length, compatible version, bit depth, rice history mult, initial history,
// rice limit, channels, max run, max frame bytes, average bit rate and
// sample rate.
void AirPlaySession::Announce() {
  const QHostAddress local = connection_->local_address();
  const QString family =
      local.protocol() == QAbstractSocket::IPv6Protocol ? "IP6" : "IP4";
  const QString sdp =
      QString(
          "v=0\r\n"
          "o=iTunes %1 0 IN %2 %3\r\n"
          "s=iTunes\r\n"
          "c=IN %2 %4\r\n"
          "t=0 0\r\n"
          "m=audio 0 RTP/AVP 96\r\n"
          "a=rtpmap:96 AppleLossless\r\n"
          "a=fmtp:96 %5 0 16 40 10 14 2 255 0 0 %6\r\n")
          .arg(connection_->session_id())
          .arg(family, local.toString(), device_.address.toString())
          .arg(AirPlayStream::kFramesPerPacket)
          .arg(AirPlayStream::kSampleRate);
  approval_timer_->start();
  connection_->Rtsp(
      "ANNOUNCE", {}, "application/sdp", sdp.toUtf8(),
      [this](const AirPlayRtspConnection::Response& r) {
        if (!CheckAllowed(r.code)) return;
        if (r.code == 403 && !connection_->encrypted()) {
          // It wants HomeKit pairing first, as AirPlay 2 receivers do.
          qLog(Info) << "AirPlay receiver" << device_.name
                     << "wants to pair first";
          state_ = State::Pairing;
          Pair();
          return;
        }
        if (r.code == 401) {
          // The connection sent the password, if there was one.
          qLog(Info) << "AirPlay receiver" << device_.name
                     << (options_.password.isEmpty() ? "wants a password"
                                                     : "refused the password");
          const bool wrong = !options_.password.isEmpty();
          Close();
          emit PasswordNeeded(wrong);
          return;
        }
        if (!Check("ANNOUNCE", r.code)) return;
        SetupAirPlay1();
      },
      options_.approval_timeout_msec);
}

// AirPlay 1's SETUP: where this sender listens for the time and for resend
// requests, answered with where the audio and sync packets go.
void AirPlaySession::SetupAirPlay1() {
  const QByteArray transport =
      QString(
          "RTP/AVP/UDP;unicast;interleaved=0-1;mode=record;control_port=%1;"
          "timing_port=%2")
          .arg(control_->localPort())
          .arg(timing_->localPort())
          .toUtf8();
  approval_timer_->start();
  connection_->Rtsp(
      "SETUP", {{"Transport", transport}}, QByteArray(), QByteArray(),
      [this](const AirPlayRtspConnection::Response& r) {
        if (!CheckAllowed(r.code)) return;
        if (!Check("SETUP", r.code)) return;
        quint16 data_port = 0;
        quint16 control_port = 0;
        for (const QByteArray& part : r.Header("Transport").split(';')) {
          const int equals = part.indexOf('=');
          const QByteArray key = part.left(equals);
          if (key == "server_port") data_port = part.mid(equals + 1).toUInt();
          if (key == "control_port") {
            control_port = part.mid(equals + 1).toUInt();
          }
        }
        if (!data_port || !control_port) {
          Fail(tr("%1 didn't say where to send audio").arg(device_.name));
          return;
        }
        // Eg. "1;timeout=60".
        connection_->set_session(r.Header("Session").split(';').value(0));
        qLog(Info) << "Set up a session with AirPlay receiver" << device_.name;
        state_ = State::SetUp;
        emit SetUp();
        // Unencrypted ALAC.
        audio_key_.clear();
        StartStream(data_port, control_port);
      },
      options_.approval_timeout_msec);
}

void AirPlaySession::Flush() {
  if (state_ != State::Ready) return;
  quint16 seq;
  quint32 rtptime;
  stream_->Flush(&seq, &rtptime);
  // Where the stream now goes on from, in time.
  stream_->SendSync();
  connection_->Rtsp(
      "FLUSH",
      {{"RTP-Info",
        QString("seq=%1;rtptime=%2").arg(seq).arg(rtptime).toUtf8()}},
      QByteArray(), QByteArray(), nullptr);
}

void AirPlaySession::SetVolume(int percent) {
  volume_ = percent;
  if (state_ != State::Ready) return;
  const double db =
      percent <= 0
          ? -144.0
          : qBound(-30.0, 20.0 * std::log10(qMin(percent, 100) / 100.0), 0.0);
  connection_->Rtsp("SET_PARAMETER", {}, "text/parameters",
                    QString("volume: %1\r\n").arg(db, 0, 'f', 6).toUtf8(),
                    nullptr);
}

void AirPlaySession::SendSync() {
  if (stream_) stream_->SendSync();
}

void AirPlaySession::SendFeedback() {
  connection_->Request("POST", "/feedback", {}, QByteArray(), QByteArray(),
                       "RTSP/1.0", nullptr);
}

// Pair-setup M5 and M6, which exchange long-term keys: this sender signs
// its pairing id and public key, and the receiver does the same, each
// encrypted with a key from the SRP exchange (HomeKit Accessory Protocol
// specification, "Pair Setup").
void AirPlaySession::PairM5(const QByteArray& srp_key) {
  namespace tlv8 = airplay::tlv8;
  const QByteArray encrypt_key = airplay::HkdfSha512(
      "Pair-Setup-Encrypt-Salt", "Pair-Setup-Encrypt-Info", srp_key);
  const QByteArray our_public =
      airplay::Ed25519PublicKey(options_.identity_seed);
  const QByteArray info =
      airplay::HkdfSha512("Pair-Setup-Controller-Sign-Salt",
                          "Pair-Setup-Controller-Sign-Info", srp_key) +
      PairingId() + our_public;
  const QByteArray sub = tlv8::Write({
      {tlv8::kIdentifier, PairingId()},
      {tlv8::kPublicKey, our_public},
      {tlv8::kSignature, airplay::Ed25519Sign(options_.identity_seed, info)},
  });
  const QByteArray m5 = tlv8::Write({
      {tlv8::kSeqNo, QByteArray(1, 5)},
      {tlv8::kEncryptedData,
       airplay::Seal(encrypt_key, QByteArray(4, '\0') + "PS-Msg05", {}, sub)},
  });
  connection_->Post(
      "/pair-setup", Headers(), m5,
      [this, srp_key, encrypt_key](const AirPlayRtspConnection::Response& r) {
        const tlv8::Items m6 = tlv8::Read(r.body);
        QByteArray plain;
        if (r.code != 200 || !tlv8::Get(m6, tlv8::kError).isNull() ||
            !airplay::Open(encrypt_key, QByteArray(4, '\0') + "PS-Msg06", {},
                           tlv8::Get(m6, tlv8::kEncryptedData), &plain)) {
          Fail(PairingError(r.code, m6));
          return;
        }
        const tlv8::Items sub = tlv8::Read(plain);
        const QByteArray id = tlv8::Get(sub, tlv8::kIdentifier);
        const QByteArray public_key = tlv8::Get(sub, tlv8::kPublicKey);
        const QByteArray info =
            airplay::HkdfSha512("Pair-Setup-Accessory-Sign-Salt",
                                "Pair-Setup-Accessory-Sign-Info", srp_key) +
            id + public_key;
        if (!airplay::Ed25519Verify(public_key,
                                    tlv8::Get(sub, tlv8::kSignature), info)) {
          Fail(tr("%1 didn't prove who it is").arg(device_.name));
          return;
        }
        qLog(Info) << "AirPlay receiver" << device_.name
                   << "will remember this sender";
        options_.receiver_pairing_id = id;
        options_.receiver_public_key = public_key;
        emit PairedPersistently(id, public_key);
        // Long-term keys are then used as on every later connection.
        PairVerify();
      });
}

// Pair-verify: an X25519 exchange, each side signing it with its long-term
// key, which gives the connection's keys without a PIN (HomeKit Accessory
// Protocol specification, "Pair Verify").
void AirPlaySession::PairVerify() {
  namespace tlv8 = airplay::tlv8;
  const QByteArray secret = airplay::RandomKey();
  const QByteArray our_public = airplay::X25519PublicKey(secret);
  connection_->Post(
      "/pair-verify", Headers(),
      tlv8::Write(
          {{tlv8::kSeqNo, QByteArray(1, 1)}, {tlv8::kPublicKey, our_public}}),
      [this, secret, our_public](const AirPlayRtspConnection::Response& r) {
        PairVerifyM3(secret, our_public, r.body);
      });
}

void AirPlaySession::PairVerifyM3(const QByteArray& secret,
                                  const QByteArray& our_public,
                                  const QByteArray& response) {
  namespace tlv8 = airplay::tlv8;
  const tlv8::Items m2 = tlv8::Read(response);
  const QByteArray their_public = tlv8::Get(m2, tlv8::kPublicKey);
  const QByteArray shared = airplay::X25519Shared(secret, their_public);
  const QByteArray key = airplay::HkdfSha512(
      "Pair-Verify-Encrypt-Salt", "Pair-Verify-Encrypt-Info", shared);
  QByteArray plain;
  bool ok = !shared.isEmpty() &&
            airplay::Open(key, QByteArray(4, '\0') + "PV-Msg02", {},
                          tlv8::Get(m2, tlv8::kEncryptedData), &plain);
  if (ok) {
    const tlv8::Items sub = tlv8::Read(plain);
    ok = tlv8::Get(sub, tlv8::kIdentifier) == options_.receiver_pairing_id &&
         airplay::Ed25519Verify(
             options_.receiver_public_key, tlv8::Get(sub, tlv8::kSignature),
             their_public + options_.receiver_pairing_id + our_public);
  }
  if (!ok) {
    qLog(Info) << "AirPlay receiver" << device_.name
               << "doesn't remember this sender";
    Close();
    emit PairingForgotten();
    return;
  }

  const QByteArray signature = airplay::Ed25519Sign(
      options_.identity_seed, our_public + PairingId() + their_public);
  const QByteArray m3 = tlv8::Write({
      {tlv8::kSeqNo, QByteArray(1, 3)},
      {tlv8::kEncryptedData,
       airplay::Seal(key, QByteArray(4, '\0') + "PV-Msg03", {},
                     tlv8::Write({{tlv8::kIdentifier, PairingId()},
                                  {tlv8::kSignature, signature}}))},
  });
  connection_->Post(
      "/pair-verify", Headers(), m3,
      [this, shared](const AirPlayRtspConnection::Response& r) {
        const tlv8::Items m4 = tlv8::Read(r.body);
        if (r.code != 200 || !tlv8::Get(m4, tlv8::kError).isNull()) {
          qLog(Info) << "AirPlay receiver" << device_.name
                     << "refused pair-verify";
          Close();
          emit PairingForgotten();
          return;
        }
        PairedWith(shared);
      });
}
