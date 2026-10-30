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

#ifndef AIRPLAY_AIRPLAYRTSPCONNECTION_H_
#define AIRPLAY_AIRPLAYRTSPCONNECTION_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <functional>
#include <memory>

#include "airplaycrypto.h"

class QTcpSocket;
class QTimer;

// The RTSP (and HTTP) connection to an AirPlay receiver, which AirPlay 2
// encrypts once pairing is done. Requests are sent one at a time, in order,
// and each response goes to its request's callback. Nothing blocks, so a
// receiver's requests on other sockets can be answered while it decides how
// to respond.
class AirPlayRtspConnection : public QObject {
  Q_OBJECT

 public:
  using Headers = QList<QPair<QByteArray, QByteArray>>;

  struct Response {
    int code = 0;
    Headers headers;
    QByteArray body;

    // The value of the header |name| (ignoring case), or a null QByteArray.
    QByteArray Header(const QByteArray& name) const;
  };
  using Callback = std::function<void(const Response&)>;

  // How long connecting may take, and how long a request waits for its
  // response by default. Receivers answer in milliseconds; a Mac that's
  // asleep never accepts the connection.
  static const int kConnectTimeoutMsec = 5000;
  static const int kTimeoutMsec = 10000;

  explicit AirPlayRtspConnection(QObject* parent = nullptr);
  ~AirPlayRtspConnection();

  void Connect(const QHostAddress& host, quint16 port);
  void Close();

  // Encrypts everything sent and received from the next request on.
  void EnableEncryption(const QByteArray& out_key, const QByteArray& in_key);
  bool encrypted() const { return cipher_ != nullptr; }

  QHostAddress local_address() const;
  quint32 session_id() const { return session_id_; }
  // An id for this sender, as hex.
  QByteArray dacp_id() const { return dacp_id_; }
  void set_dacp_id(const QByteArray& id) { dacp_id_ = id; }
  // What receivers show for this sender, sent with every request. A Mac
  // shows it when it asks whether to allow a sender, and remembers its
  // answer by it.
  void set_client_name(const QString& name) { client_name_ = name.toUtf8(); }
  // For tests, which don't want to wait as long.
  void set_timeouts(int connect_msec, int request_msec);
  // The URI of RTSP requests: rtsp://<local address>/<session id>.
  QByteArray uri() const;

  // Queues a request. |uri| empty means uri(). Fails the connection if
  // there's no response in |timeout_msec|, or the default if it's negative.
  void Request(const QByteArray& method, const QByteArray& uri,
               const Headers& headers, const QByteArray& content_type,
               const QByteArray& body, const QByteArray& protocol,
               Callback callback, int timeout_msec = -1);
  void Rtsp(const QByteArray& method, const Headers& headers,
            const QByteArray& content_type, const QByteArray& body,
            Callback callback, int timeout_msec = -1);
  // An HTTP POST of application/octet-stream to |path|.
  void Post(const QByteArray& path, const Headers& headers,
            const QByteArray& body, Callback callback, int timeout_msec = -1);

 signals:
  void Connected();
  // The connection failed or closed, or a request timed out.
  void Failed(const QString& error);

 private slots:
  void SocketError();
  void ReadyRead();
  void ConnectTimedOut();
  void TimedOut();

 private:
  // A request, built into a message when it's sent: the CSeq and the
  // encryption can change while it's queued.
  struct Pending {
    QByteArray method;
    QByteArray uri;
    Headers headers;
    QByteArray content_type;
    QByteArray body;
    QByteArray protocol;
    Callback callback;
    int timeout_msec = kTimeoutMsec;
  };

  void NewSocket();
  void SendNext();
  bool ParseResponse(Response* response);

  QTcpSocket* socket_;
  QHostAddress host_;
  quint16 port_;
  QTimer* connect_timeout_;
  QTimer* timeout_;
  int request_timeout_msec_;
  std::unique_ptr<airplay::FrameCipher> cipher_;
  QQueue<Pending> queue_;
  bool in_flight_;
  QByteArray buffer_;
  int cseq_;
  const quint32 session_id_;
  QByteArray dacp_id_;
  const quint32 active_remote_;
  QByteArray client_name_;
};

#endif  // AIRPLAY_AIRPLAYRTSPCONNECTION_H_
