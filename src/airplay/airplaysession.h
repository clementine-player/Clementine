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

#ifndef AIRPLAY_AIRPLAYSESSION_H_
#define AIRPLAY_AIRPLAYSESSION_H_

#include <QHostAddress>
#include <QObject>
#include <memory>

#include "airplaycrypto.h"
#include "airplaydevice.h"
#include "airplayrtspconnection.h"
#include "airplaytlv8.h"

class AirPlayRtspConnection;
class AirPlayStream;
class QTcpSocket;
class QTimer;
class QUdpSocket;

// A connection to one AirPlay 2 receiver: GET /info, then HomeKit transient
// pairing (pair-setup M1 to M4 with the fixed PIN 3939), after which
// everything on the connection is encrypted, then the first SETUP, which
// says who this is and where to ask for the time. Runs on the event loop.
//
// A receiver may ask its user whether to allow a sender it doesn't know, and
// hold that SETUP until they answer. A Mac remembers the answer by the
// sender's name, so Open() sends the same name each time. (POST
// /pair-pin-start isn't sent: it makes a receiver show a PIN to pair with,
// which transient pairing doesn't use.)
//
// Then it sets up a stream of realtime audio: an event channel, which the
// receiver sends requests on, a second SETUP with the stream's key and
// control port, RECORD and FLUSH.
//
// Receivers ask the sender for the time while setting up, and may not
// answer SETUP until they've had an answer, so that's answered from the
// start. While streaming, sync packets go every second and POST /feedback
// every two, which keeps the session alive.
class AirPlaySession : public QObject {
  Q_OBJECT

 public:
  struct Options {
    // What the receiver shows for this sender.
    QString name;
    // Who this sender is, in the form of a MAC address. A Mac remembers
    // which senders its user allowed by this and the name, so it should stay
    // the same: see SavedSenderId(). Empty for a new one each time.
    QString sender_id;
    // The receiver's password, which is the PIN of transient pairing, or
    // empty for the fixed PIN 3939.
    QString password;
    // For pairing that the receiver remembers, with long-term keys: this
    // sender's Ed25519 seed, or empty for transient pairing every time. And
    // what was learned of the receiver by pairing before, if anything: its
    // pairing id and Ed25519 public key, to pair-verify with.
    QByteArray identity_seed;
    QByteArray receiver_pairing_id;
    QByteArray receiver_public_key;
    // Whether to ask the receiver to show a PIN, which SubmitPin() then
    // pairs with: an Apple TV needs one.
    bool show_pin;
    // How long to wait: for the connection, for each response, and for the
    // first SETUP, which waits for the receiver's user to allow this sender.
    int connect_timeout_msec;
    int request_timeout_msec;
    int approval_timeout_msec;

    Options();
  };

  explicit AirPlaySession(const Options& options, QObject* parent = nullptr);
  ~AirPlaySession();

  // "Clementine on <this computer's name>".
  static QString DefaultName();
  // A random sender id, made the first time and kept in the settings.
  static QString SavedSenderId();
  // This sender's long-term Ed25519 seed, likewise.
  static QByteArray SavedIdentitySeed();

  // Connects to |device|, pairs and sets up the stream. Paired, SetUp and
  // Ready, or Failed follows.
  void Open(const AirPlayDevice& device);
  // Tears the session down, if it was set up.
  void Close();
  // For the next Open().
  void set_options(const Options& options) { options_ = options; }
  // Pairs with the PIN the receiver showed, after PinShown.
  void SubmitPin(const QString& pin);

  bool is_open() const { return state_ != State::Closed; }
  bool is_ready() const { return state_ == State::Ready; }
  // Where the audio goes, while ready.
  AirPlayStream* stream() const { return stream_; }

  // Tells the receiver to drop what it has buffered and play the stream
  // from where it goes on, after a seek or a pause.
  void Flush();
  // The gain to play at, from 0 to 100 percent, as Clementine's engines take
  // it once its volume slider's curve is applied: so it sounds as loud as
  // playing locally. AirPlay takes it in dB, from -30 to 0, or -144 for
  // silence.
  void SetVolume(int percent);

 signals:
  void Paired();
  // The receiver wants a password, or the one given was wrong. The session
  // has closed; open it again with the password.
  void PasswordNeeded(bool wrong);
  // The receiver is showing a PIN to pair with: see SubmitPin().
  void PinShown();
  // The receiver will remember this sender by these, for pair-verify next
  // time.
  void PairedPersistently(const QByteArray& receiver_pairing_id,
                          const QByteArray& receiver_public_key);
  // Pair-verify failed: the receiver doesn't remember this sender any more.
  // The session has closed; forget what was kept and pair again.
  void PairingForgotten();
  // The receiver is taking a while to answer the first SETUP, probably
  // because it's asking its user whether to allow this sender.
  void WaitingForApproval();
  void SetUp();
  // Audio written to stream() now plays.
  void Ready();
  // stream() is about to be deleted, so nothing should write to it.
  void StreamEnding();
  // The receiver's user didn't allow this sender, or didn't answer, which
  // they may yet: a message for the user.
  void NotAllowed(const QString& message);
  // A message for the user.
  void Failed(const QString& error);

 private slots:
  void Connected();
  void Fail(const QString& error);
  void TimingReadyRead();
  void SendFeedback();
  void SendSync();
  void ConnectEventChannel();
  void EventConnected();
  void EventError();
  void EventReadyRead();

 private:
  enum class State { Closed, Connecting, Pairing, SettingUp, SetUp, Ready };

  void GetInfo();
  void Pair();
  void PairSetup();
  void PairM3(const QByteArray& response);
  void PairM5(const QByteArray& srp_key);
  void PairVerify();
  void PairVerifyM3(const QByteArray& secret, const QByteArray& our_public,
                    const QByteArray& response);
  // Encrypts the connection with keys derived from |shared|, then sets up.
  void PairedWith(const QByteArray& shared);
  // The PIN pair-setup uses.
  QString Pin() const;
  AirPlayRtspConnection::Headers Headers() const;
  QByteArray PairingId() const;
  bool persistent() const { return !options_.identity_seed.isEmpty(); }
  void SetupSession();
  void SetupStream();
  void StartStream(quint16 data_port, quint16 control_port);
  // False, after failing, unless |code| is 200.
  bool Check(const QByteArray& what, int code);
  // What a pairing step's error TLV means, for the user.
  QString PairingError(int code, const airplay::tlv8::Items& items) const;

  Options options_;
  AirPlayDevice device_;
  AirPlayRtspConnection* connection_;
  State state_;
  // Where the receiver asks for the time.
  QUdpSocket* timing_;
  // Notices that the first SETUP is waiting for the receiver's user.
  QTimer* approval_timer_;
  QByteArray shared_key_;

  // The control channel, where sync packets go from and resend requests
  // come to.
  QUdpSocket* control_;
  QTimer* sync_timer_;
  QTimer* feedback_timer_;

  QTcpSocket* event_socket_;
  std::unique_ptr<airplay::FrameCipher> event_cipher_;
  QByteArray event_buffer_;
  quint16 event_port_;
  int event_attempts_;

  QByteArray audio_key_;
  AirPlayStream* stream_;
  int volume_;
};

#endif  // AIRPLAY_AIRPLAYSESSION_H_
