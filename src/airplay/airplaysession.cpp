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
#include <QTimer>
#include <QUdpSocket>
#include <QUuid>
#include <QVariantMap>
#include <QtEndian>
#include <chrono>

#include "airplaycrypto.h"
#include "airplayplist.h"
#include "airplayrtspconnection.h"
#include "airplaysrp.h"
#include "core/logging.h"

namespace {

// The PIN of transient pairing, which receivers that don't need a password
// take, as pyatv and Apple's senders use. A Mac shows it as its PIN when
// asked to with /pair-pin-start.
const char kTransientPin[] = "3939";

// X-Apple-HKP: 4 asks for transient HomeKit pairing, as pyatv sends it.
const AirPlayRtspConnection::Headers kPairingHeaders = {
    {"X-Apple-HKP", "4"}, {"Connection", "keep-alive"}};

const char kPlistType[] = "application/x-apple-binary-plist";
const char kSettingsGroup[] = "AirPlay";

// How long the first SETUP takes before it's probably waiting for the
// receiver's user.
const int kApprovalNoticeMsec = 1000;

// Now, as a 64-bit NTP timestamp: seconds since 1900, then the fraction.
quint64 NtpNow() {
  using namespace std::chrono;
  const quint64 us =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();
  // Seconds from 1900 to 1970 (RFC 5905).
  const quint64 secs = us / 1000000 + 0x83AA7E80;
  const quint64 frac = ((us % 1000000) << 32) / 1000000;
  return (secs << 32) | frac;
}

}  // namespace

AirPlaySession::Options::Options()
    : name(DefaultName()),
      connect_timeout_msec(AirPlayRtspConnection::kConnectTimeoutMsec),
      request_timeout_msec(AirPlayRtspConnection::kTimeoutMsec),
      // A Mac holds the first SETUP until its user answers, which can take
      // longer than other responses may. A Mac gives up after 15 s and
      // answers 400; others may wait longer.
      approval_timeout_msec(30000) {}

AirPlaySession::AirPlaySession(const Options& options, QObject* parent)
    : QObject(parent),
      options_(options),
      connection_(new AirPlayRtspConnection(this)),
      state_(State::Closed),
      timing_(new QUdpSocket(this)),
      approval_timer_(new QTimer(this)) {
  connection_->set_client_name(options_.name);
  connection_->set_timeouts(options_.connect_timeout_msec,
                            options_.request_timeout_msec);
  connect(connection_, SIGNAL(Connected()), SLOT(Connected()));
  connect(connection_, SIGNAL(Failed(QString)), SLOT(Fail(QString)));
  connect(timing_, SIGNAL(readyRead()), SLOT(TimingReadyRead()));
  approval_timer_->setSingleShot(true);
  approval_timer_->setInterval(kApprovalNoticeMsec);
  connect(approval_timer_, SIGNAL(timeout()), SIGNAL(WaitingForApproval()));
}

AirPlaySession::~AirPlaySession() {}

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

QString AirPlaySession::DefaultName() {
  return tr("Clementine on %1").arg(QSysInfo::machineHostName());
}

void AirPlaySession::Open(const AirPlayDevice& device) {
  Close();
  device_ = device;
  state_ = State::Connecting;
  qLog(Info) << "Connecting to AirPlay receiver" << device;
  connection_->Connect(device.address, device.port);
}

void AirPlaySession::Close() {
  if (state_ == State::SetUp) {
    // Sent, but not waited for.
    connection_->Rtsp("TEARDOWN", {}, QByteArray(), QByteArray(), nullptr);
  }
  state_ = State::Closed;
  approval_timer_->stop();
  connection_->Close();
  timing_->close();
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
  if (!timing_->bind(connection_->local_address(), 0)) {
    Fail(tr("Couldn't listen for the AirPlay receiver"));
    return;
  }
  GetInfo();
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
              AirPlayDevice::PasswordRequired) {
            Fail(tr("%1 requires a password, which Clementine can't use yet")
                     .arg(device_.name));
            return;
          }
        }
        Pair();
      });
}

// HomeKit transient pairing: pair-setup M1 to M4 with the fixed PIN.
void AirPlaySession::Pair() {
  using airplay::tlv8::Write;
  const QByteArray m1 = Write({
      {airplay::tlv8::kMethod, QByteArray(1, '\0')},
      {airplay::tlv8::kSeqNo, QByteArray(1, 1)},
      {airplay::tlv8::kFlags,
       QByteArray(1, char(airplay::tlv8::kFlagTransient))},
  });
  connection_->Post("/pair-setup", kPairingHeaders, m1,
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
      airplay::SrpClient().Process(kTransientPin, salt, b_public);
  const QByteArray m3 = tlv8::Write({
      {tlv8::kSeqNo, QByteArray(1, 3)},
      {tlv8::kPublicKey, srp.public_key},
      {tlv8::kProof, srp.proof},
  });
  const QByteArray key = srp.key;
  connection_->Post(
      "/pair-setup", kPairingHeaders, m3,
      [this, key](const AirPlayRtspConnection::Response& r) {
        const tlv8::Items m4 = tlv8::Read(r.body);
        if (r.code != 200 || !tlv8::Get(m4, tlv8::kError).isNull()) {
          Fail(PairingError(r.code, m4));
          return;
        }
        // The keys of the encrypted connection, as HomeKit derives them.
        connection_->EnableEncryption(
            airplay::HkdfSha512("Control-Salt", "Control-Write-Encryption-Key",
                                key),
            airplay::HkdfSha512("Control-Salt", "Control-Read-Encryption-Key",
                                key));
        qLog(Info) << "Paired with AirPlay receiver" << device_.name;
        state_ = State::SettingUp;
        emit Paired();
        SetupSession();
      });
}

QString AirPlaySession::PairingError(int code,
                                     const airplay::tlv8::Items& items) const {
  namespace tlv8 = airplay::tlv8;
  const QByteArray error = tlv8::Get(items, tlv8::kError);
  // HAP's kTLVError values.
  switch (error.isEmpty() ? -1 : quint8(error[0])) {
    case 0x02:  // Authentication: the SRP proof didn't match.
      return tr("%1 refused to pair. It may want a password, which "
                "Clementine can't use yet.")
          .arg(device_.name);
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
        // A Mac answers 400 when its user doesn't allow this sender, or
        // doesn't answer in 15 s.
        const bool asked = !approval_timer_->isActive();
        approval_timer_->stop();
        if (r.code == 400 && asked) {
          Fail(tr("%1 didn't allow %2 to play there")
                   .arg(device_.name, options_.name));
          return;
        }
        if (r.code != 200) {
          Fail(tr("%1 refused to set up a session (HTTP %2)")
                   .arg(device_.name)
                   .arg(r.code));
          return;
        }
        qLog(Info) << "Set up a session with AirPlay receiver" << device_.name;
        state_ = State::SetUp;
        emit SetUp();
      },
      options_.approval_timeout_msec);
}

// Answers the receiver's NTP-style requests for the time.
void AirPlaySession::TimingReadyRead() {
  while (timing_->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = timing_->receiveDatagram();
    const QByteArray data = datagram.data();
    if (data.size() < 32) continue;
    const quint64 now = NtpNow();
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
