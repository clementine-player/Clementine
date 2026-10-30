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

#include "airplaydevice.h"
#include "airplaytlv8.h"

class AirPlayRtspConnection;
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
// Receivers ask the sender for the time while setting up, and may not
// answer SETUP until they've had an answer, so that's answered from the
// start.
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

  // Connects to |device|, pairs and sends the first SETUP. Paired, then
  // SetUp, or Failed follows.
  void Open(const AirPlayDevice& device);
  // Tears the session down, if it was set up.
  void Close();

  bool is_open() const { return state_ != State::Closed; }
  bool is_set_up() const { return state_ == State::SetUp; }

 signals:
  void Paired();
  // The receiver is taking a while to answer the first SETUP, probably
  // because it's asking its user whether to allow this sender.
  void WaitingForApproval();
  void SetUp();
  // A message for the user.
  void Failed(const QString& error);

 private slots:
  void Connected();
  void Fail(const QString& error);
  void TimingReadyRead();

 private:
  enum class State { Closed, Connecting, Pairing, SettingUp, SetUp };

  void GetInfo();
  void Pair();
  void PairM3(const QByteArray& response);
  void SetupSession();
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
};

#endif  // AIRPLAY_AIRPLAYSESSION_H_
