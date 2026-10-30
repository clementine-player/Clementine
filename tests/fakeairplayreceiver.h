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

#ifndef TESTS_FAKEAIRPLAYRECEIVER_H_
#define TESTS_FAKEAIRPLAYRECEIVER_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QStringList>
#include <functional>
#include <memory>
#include <optional>

#include "airplay/airplaycrypto.h"
#include "airplay/airplaydevice.h"

class QTcpServer;
class QTcpSocket;
class QUdpSocket;

// An AirPlay 2 receiver on localhost that behaves as a Mac (AirTunes
// 980.77.5) was seen to, for testing the sender against:
//
// - GET /info gives its status flags, with 0x200 while a password is set.
// - Transient pair-setup with SRP, using the password as the PIN (or 3939).
//   A wrong PIN gets M4 with error 2.
// - Then everything on the connection is encrypted.
// - It asks its user whether to allow each sender name it doesn't know,
//   holding the first SETUP until Allow() is called. Setting a password
//   forgets the allowed names.
// - It asks the sender for the time while setting up, and doesn't answer
//   the SETUP that starts the stream until the sender has answered.
//
// It decrypts the audio and keeps it, and notes mistakes such as a reused
// nonce or an audio key that's one of the event channel's keys.
//
// set_airplay1() makes it an AirPlay 1 receiver instead, which takes
// ANNOUNCE, a SETUP with a Transport header and unencrypted ALAC, and asks
// to allow a sender at ANNOUNCE. With a password, it asks for HTTP Digest
// authentication as an AirPort Express does, unless it wants pairing first.
class FakeAirPlayReceiver : public QObject {
 public:
  FakeAirPlayReceiver();
  ~FakeAirPlayReceiver();

  // Listens on 127.0.0.1. False if it couldn't.
  bool Listen();
  // How the sender would find it.
  AirPlayDevice device() const;

  // Empty for none. Setting one forgets the allowed names, as a Mac did.
  void SetPassword(const QString& password);
  // A PIN it shows when asked with /pair-pin-start, and pairs with then, as
  // an Apple TV does. Empty for none.
  void set_pin(const QString& pin) { pin_ = pin; }
  // Forgets the senders it paired with persistently.
  void ForgetSenders() { senders_.clear(); }
  // The senders it remembers: pairing id to Ed25519 public key.
  QMap<QByteArray, QByteArray> senders() const { return senders_; }
  static QByteArray PairingId() { return "fake-receiver"; }
  // Its long-term public key.
  static QByteArray PublicKey();

  // Whether it's an AirPlay 1 receiver.
  void set_airplay1(bool airplay1) { airplay1_ = airplay1; }
  // Whether, as an AirPlay 1 receiver, it refuses ANNOUNCE with 403 until
  // the sender has paired, with the password as the PIN.
  void set_airplay1_pairing(bool pairing) { airplay1_pairing_ = pairing; }

  // Whether its status flags say it has a password. A Mac kept wanting one
  // after saying it didn't.
  void set_report_password(bool report) { report_password_ = report; }
  // Whether it asks its user to allow new sender names.
  void set_ask_to_allow(bool ask) { ask_to_allow_ = ask; }
  // Called when it asks its user whether to allow a sender name.
  void set_on_prompt(std::function<void(const QString&)> on_prompt) {
    on_prompt_ = on_prompt;
  }
  // Answers the pending prompt with Allow, or with Deny, which a Mac also
  // does when its user doesn't answer in 15 s.
  void Allow();
  void Deny();

  int prompts() const { return prompts_; }
  bool paired() const { return paired_; }
  // The requests it saw: the method, and the path unless it's an RTSP URI.
  QStringList requests() const { return requests_; }
  // The sender names it saw.
  QSet<QString> client_names() const { return client_names_; }
  int timing_replies() const { return timing_replies_; }
  // SETUPs it held until the sender answered a request for the time.
  int setups_after_timing() const { return setups_after_timing_; }
  // The audio it decoded, as S16LE stereo.
  QByteArray audio() const { return audio_; }
  int audio_packets() const { return audio_packets_; }
  int sync_packets() const { return sync_packets_; }
  int event_replies() const { return event_replies_; }
  // FLUSHes after the first, and the last volume set, in dB.
  int flushes() const { return flushes_; }
  QByteArray volume() const { return volume_; }
  // Packets it dropped as too late to play, or from before a FLUSH.
  int late_packets() const { return late_packets_; }
  int dropped_packets() const { return dropped_packets_; }
  // What the first SETUP said the sender's MAC address was.
  QString sender_mac_address() const { return sender_mac_address_; }
  bool torn_down() const { return torn_down_; }
  // Requests a sender shouldn't make.
  QStringList errors() const { return errors_; }

 private:
  using Headers = QList<QPair<QByteArray, QByteArray>>;

  struct Request {
    QByteArray method;
    QByteArray uri;
    Headers headers;
    QByteArray body;

    QByteArray Header(const QByteArray& name) const;
  };

  void NewConnection();
  void ReadyRead();
  // Answers requests until one has to wait.
  void ProcessRequests();
  bool ParseRequest(Request* request);
  // Handles |request|, or returns false to hold it.
  bool Handle(const Request& request);
  void Respond(const Request& request, int code, const QByteArray& body = {},
               const Headers& headers = {});
  // Handles |request| as an AirPlay 1 receiver does, returning false to hold
  // it, or nullopt for one that's handled as an AirPlay 2 receiver does.
  std::optional<bool> HandleAirPlay1(const Request& request);
  // Whether |request| has the password, by HTTP Digest, if one's needed.
  bool Authorized(const Request& request) const;
  // Appends the PCM of an uncompressed ALAC frame to audio_.
  void DecodeAlac(const QByteArray& frame);
  // False if |request| has to wait for its sender name to be allowed.
  bool CheckAllowed(const Request& request);
  void PairSetup(const Request& request);
  void PairSetupM5(const Request& request);
  void PairVerify(const Request& request);
  // Encrypts the connection with keys from |shared|.
  void Encrypt(const QByteArray& shared);
  bool SetupStream(const Request& request);
  void SendTimingRequest();
  void TimingReadyRead();
  void ControlReadyRead();
  void AudioReadyRead();
  void NewEventConnection();
  void EventReadyRead();
  quint32 status_flags() const;

  QTcpServer* server_;
  QTcpSocket* client_;
  QByteArray buffer_;
  std::optional<Request> held_;
  std::unique_ptr<airplay::FrameCipher> cipher_;
  bool paired_;

  QString password_;
  bool airplay1_;
  bool airplay1_pairing_;
  QByteArray digest_nonce_;
  bool ask_to_allow_;
  bool report_password_;
  QSet<QString> allowed_names_;
  bool prompting_;
  QString prompted_name_;
  int prompts_;
  std::function<void(const QString&)> on_prompt_;

  // SRP, between M2 and M4.
  QByteArray srp_salt_;
  QByteArray srp_b_;
  QByteArray srp_b_public_;

  QString sender_mac_address_;
  QUdpSocket* timing_;
  quint16 sender_timing_port_;
  int timing_requests_;
  int timing_replies_;
  bool torn_down_;

  QByteArray shared_key_;
  QString pin_;
  bool showing_pin_;
  QByteArray srp_key_;
  QMap<QByteArray, QByteArray> senders_;
  // Pair-verify, between M2 and M4.
  QByteArray verify_shared_;
  QByteArray verify_their_public_;
  QByteArray verify_our_public_;
  QTcpServer* event_server_;
  QTcpSocket* event_socket_;
  std::unique_ptr<airplay::FrameCipher> event_cipher_;
  QByteArray event_buffer_;
  QUdpSocket* control_;
  QUdpSocket* audio_socket_;
  bool waiting_for_time_;
  int timing_replies_before_setup_;
  int setups_after_timing_;
  QByteArray audio_key_;
  QSet<QByteArray> audio_nonces_;
  QByteArray audio_;
  int audio_packets_;
  int sync_packets_;
  int event_replies_;
  int flushes_;
  bool flushed_once_;
  int flush_seq_;
  bool have_sync_;
  quint32 sync_rtptime_;
  quint64 sync_ntp_;
  int late_packets_;
  int dropped_packets_;
  int last_seq_;
  quint32 last_rtptime_;
  QByteArray volume_;
  QStringList errors_;

  QStringList requests_;
  QSet<QString> client_names_;
};

#endif  // TESTS_FAKEAIRPLAYRECEIVER_H_
