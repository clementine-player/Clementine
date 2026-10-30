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

#ifndef AIRPLAY_RTSPCONNECTION_H_
#define AIRPLAY_RTSPCONNECTION_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QTcpSocket>
#include <QTimer>
#include <functional>
#include <memory>

#include "crypto.h"

namespace airplay {

using Headers = QList<QPair<QByteArray, QByteArray>>;

// The RTSP (and HTTP) connection to a receiver, which AirPlay 2 encrypts once
// pairing is done. Requests are sent one at a time, in order, and each
// response goes to its request's callback. Nothing blocks, so the receiver's
// UDP requests are answered while it decides how to respond.
class RtspConnection : public QObject {
  Q_OBJECT

 public:
  struct Response {
    int code = 0;
    Headers headers;
    QByteArray body;

    // The value of the header |name| (ignoring case), or a null QByteArray.
    QByteArray Header(const QByteArray& name) const;
  };
  using Callback = std::function<void(const Response&)>;

  // How long a request waits for its response by default, and how long
  // connecting may take.
  static const int kTimeoutMsec = 10000;
  static const int kConnectTimeoutMsec = 5000;

  explicit RtspConnection(QObject* parent = nullptr);

  void Connect(const QHostAddress& host, quint16 port);

  // Encrypts everything sent and received from the next request on.
  void EnableEncryption(const QByteArray& out_key, const QByteArray& in_key);

  QHostAddress local_address() const { return socket_->localAddress(); }
  QHostAddress peer_address() const { return socket_->peerAddress(); }
  quint32 session_id() const { return session_id_; }
  // An id for this sender, as hex: random unless set.
  QByteArray dacp_id() const { return dacp_id_; }
  void set_dacp_id(const QByteArray& id) { dacp_id_ = id; }
  // What receivers show for this sender, sent with every request.
  void set_client_name(const QString& name) { client_name_ = name.toUtf8(); }
  // For receivers that ask for HTTP Digest authentication (AirPlay 1).
  void set_password(const QString& password) { password_ = password.toUtf8(); }
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
  // A request, built into a message when it's sent: the CSeq, encryption and
  // authentication can change while it's queued.
  struct Pending {
    QByteArray method;
    QByteArray uri;
    Headers headers;
    QByteArray content_type;
    QByteArray body;
    QByteArray protocol;
    Callback callback;
    int timeout_msec = kTimeoutMsec;
    // Whether it was already resent with a digest.
    bool authenticated = false;
  };

  void SendNext();
  // True if the request at the head of the queue should be resent answering
  // the Digest challenge in |response|.
  bool Authenticate(const Response& response);
  QByteArray DigestAuthorization(const QByteArray& method,
                                 const QByteArray& uri) const;
  bool ParseResponse(Response* response);

  QTcpSocket* socket_;
  QHostAddress host_;
  quint16 port_ = 0;
  QTimer* connect_timeout_;
  int request_timeout_msec_ = kTimeoutMsec;
  QTimer* timeout_;
  std::unique_ptr<FrameCipher> cipher_;
  QQueue<Pending> queue_;
  bool in_flight_ = false;
  QByteArray buffer_;
  int cseq_ = 0;
  quint32 session_id_;
  QByteArray dacp_id_;
  QByteArray client_name_;
  QByteArray password_;
  QByteArray digest_realm_;
  QByteArray digest_nonce_;
  quint32 active_remote_;
};

}  // namespace airplay

#endif  // AIRPLAY_RTSPCONNECTION_H_
