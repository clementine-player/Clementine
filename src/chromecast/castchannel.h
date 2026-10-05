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

#ifndef CHROMECAST_CASTCHANNEL_H_
#define CHROMECAST_CASTCHANNEL_H_

#include <QElapsedTimer>
#include <QHostAddress>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QSet>
#include <QTimer>

class QSslSocket;

// A connection to a Cast device.
//
// It's TLS to the device's port (8009, or a speaker group's own port),
// carrying CastMessages, each preceded by its size as a 32-bit big-endian
// integer. Every message is addressed from a sender id to a receiver id
// within a namespace, and Clementine's all carry JSON.
//
// Before talking to an id, a sender opens a virtual connection to it with a
// CONNECT message; Send does that the first time it's needed. Both sides
// PING each other and answer with PONG. A device that goes quiet for longer
// than three heartbeats is treated as gone.
//
// The device's certificate isn't checked. Cast devices present self-signed
// ones, and senders authenticate devices with a separate challenge that
// Clementine doesn't make, so a device on the local network could pretend to
// be a Cast device. It would only be told what to play.
class CastChannel : public QObject {
  Q_OBJECT

 public:
  static const char* kConnectionNamespace;
  static const char* kHeartbeatNamespace;
  // The id of the device itself, as opposed to the apps running on it.
  static const char* kReceiverId;
  static const char* kSenderId;
  // Cast devices don't accept larger messages.
  static const int kMaxMessageSize;

  // One decoded message, without the protobuf.
  struct Message {
    QString source_id;
    QString destination_id;
    QString name_space;
    // Empty for a binary payload.
    QString payload;
  };

  // Prefixes a serialized CastMessage carrying |message| with its size.
  static QByteArray Encode(const Message& message);
  // Removes every complete message from the front of |buffer|. Returns false
  // if the data can't be a Cast message: too large, or not a CastMessage.
  static bool Decode(QByteArray* buffer, QList<Message>* messages);

  explicit CastChannel(QObject* parent = nullptr);
  ~CastChannel();

  // Connects, and once that's done, opens a virtual connection to the
  // device and emits Opened.
  void Open(const QHostAddress& address, quint16 port);
  // Closes the connection without emitting Closed.
  void Close();
  bool is_open() const { return open_; }

  // Sends |payload| to |destination|, opening a virtual connection to it
  // first if this channel doesn't have one.
  void Send(const QString& name_space, const QString& destination,
            const QJsonObject& payload);

  // How often to PING. Tests make it shorter.
  void set_heartbeat_interval(int msec);

 signals:
  void Opened();
  // The connection failed or ended, other than by Close().
  void Closed(const QString& reason);
  void MessageReceived(const QString& name_space, const QString& source_id,
                       const QJsonObject& payload);

 private slots:
  void Encrypted();
  void ReadyRead();
  void SocketError();
  void Heartbeat();

 private:
  void SendRaw(const QString& name_space, const QString& destination,
               const QJsonObject& payload);
  void Handle(const Message& message);
  void Fail(const QString& reason);

  QSslSocket* socket_;
  bool open_;
  QByteArray buffer_;
  QSet<QString> connected_ids_;
  QTimer heartbeat_;
  // Since anything last arrived.
  QElapsedTimer last_received_;
};

#endif  // CHROMECAST_CASTCHANNEL_H_
