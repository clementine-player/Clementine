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

#ifndef AIRPLAY_FAKERECEIVER_H_
#define AIRPLAY_FAKERECEIVER_H_

#include <QByteArray>
#include <QHostAddress>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <memory>
#include <optional>

#include "crypto.h"
#include "rtspconnection.h"

class QTcpServer;
class QTcpSocket;
class QUdpSocket;

namespace airplay {

// An AirPlay receiver on localhost that behaves as a Mac (AirTunes 980.77.5)
// was seen to, for testing the sender against:
//
// - GET /info gives its status flags, with 0x200 while a password is set.
// - It asks its user whether to allow each sender name it doesn't know,
//   holding pair-pin-start (or ANNOUNCE) open until Allow() is called.
//   Setting a password forgets the allowed names.
// - Transient pair-setup with SRP, using the password as the PIN (or 3939).
//   A wrong PIN gives M4 with error 2.
// - Then everything on the connection is encrypted.
// - It asks the sender for the time while setting up, and doesn't answer
//   SETUP until the sender has answered.
// - AirPlay 1: with a password, ANNOUNCE gets 403 until the sender has
//   paired. A legacy receiver asks for HTTP Digest instead.
//
// It decrypts and keeps the audio, and notes mistakes such as nonce reuse.
class FakeReceiver : public QObject {
  Q_OBJECT

 public:
  struct Options {
    // The status flags it reports without a password: a Mac reported 0x4,
    // then 0x84.
    quint32 status_flags = 0x84;
    // Whether it asks its user to allow new sender names.
    bool ask_to_allow = true;
    // An older AirPlay 1 receiver: HTTP Digest for its password, no pairing.
    bool legacy_digest = false;
  };

  explicit FakeReceiver(const Options& options, QObject* parent = nullptr);
  ~FakeReceiver();

  // Listens on 127.0.0.1. False if it couldn't.
  bool Listen();
  QHostAddress address() const { return QHostAddress::LocalHost; }
  quint16 port() const;

  // Empty for none. Setting one forgets the allowed names, as a Mac did.
  void SetPassword(const QString& password);
  // What it advertises in its _airplay._tcp TXT record.
  QMap<QString, QString> txt() const;

  // Answers the pending prompt with Allow.
  void Allow();
  QSet<QString> allowed_names() const { return allowed_names_; }
  int prompts() const { return prompts_; }

  // What it saw.
  QStringList requests() const { return requests_; }
  // The audio it decoded, as S16LE stereo.
  QByteArray audio() const { return audio_; }
  int audio_packets() const { return audio_packets_; }
  int timing_requests() const { return timing_requests_; }
  int timing_replies() const { return timing_replies_; }
  // SETUPs it held until the sender answered a request for the time.
  int setups_after_timing() const { return setups_after_timing_; }
  int sync_packets() const { return sync_packets_; }
  int event_replies() const { return event_replies_; }
  bool torn_down() const { return torn_down_; }
  // Protocol mistakes by the sender, such as a nonce used twice.
  QStringList errors() const { return errors_; }

 signals:
  // It's asking its user whether to allow |name|.
  void Prompted(const QString& name);
  void TornDown();

 private slots:
  void NewConnection();
  void ReadyRead();
  void NewEventConnection();
  void EventReadyRead();
  void TimingReadyRead();
  void ControlReadyRead();
  void AudioReadyRead();

 private:
  struct Request {
    QByteArray method;
    QByteArray uri;
    Headers headers;
    QByteArray body;

    QByteArray Header(const QByteArray& name) const;
  };

  // Answers requests until one has to wait.
  void ProcessRequests();
  bool ParseRequest(Request* request);
  // Handles |request|, or returns false to hold it.
  bool Handle(const Request& request);
  void Respond(const Request& request, int code, const QByteArray& body = {},
               const Headers& headers = {});
  // False, after answering 401, unless |request| has a valid digest.
  bool CheckDigest(const Request& request);
  // False if |request| has to wait for its sender name to be allowed.
  bool CheckAllowed(const Request& request);

  void PairSetup(const Request& request);
  bool Setup(const Request& request);
  void SendTimingRequest();
  void SendEvent();

  Options options_;
  QString password_;
  QSet<QString> allowed_names_;
  bool prompting_ = false;
  QByteArray prompted_name_;
  int prompts_ = 0;

  QTcpServer* server_;
  QTcpSocket* client_ = nullptr;
  QByteArray buffer_;
  // The request being handled, while it waits.
  std::optional<Request> held_;
  std::unique_ptr<FrameCipher> cipher_;
  bool paired_ = false;
  QByteArray digest_nonce_;

  // SRP, between M2 and M4.
  QByteArray srp_salt_;
  QByteArray srp_b_;
  QByteArray srp_b_public_;
  QByteArray shared_key_;

  QTcpServer* event_server_;
  QTcpSocket* event_socket_ = nullptr;
  std::unique_ptr<FrameCipher> event_cipher_;
  QByteArray event_buffer_;

  QUdpSocket* timing_;
  QUdpSocket* control_;
  QUdpSocket* audio_socket_;
  quint16 sender_timing_port_ = 0;
  bool waiting_for_time_ = false;
  int timing_replies_before_setup_ = 0;
  int setups_after_timing_ = 0;

  QByteArray audio_key_;
  QSet<QByteArray> audio_nonces_;
  QByteArray audio_;
  int audio_packets_ = 0;

  QStringList requests_;
  QStringList errors_;
  int timing_requests_ = 0;
  int timing_replies_ = 0;
  int sync_packets_ = 0;
  int event_replies_ = 0;
  bool torn_down_ = false;
};

}  // namespace airplay

#endif  // AIRPLAY_FAKERECEIVER_H_
