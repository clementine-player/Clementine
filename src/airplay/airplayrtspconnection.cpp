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

#include "airplayrtspconnection.h"

#include <QRandomGenerator>
#include <QStringList>
#include <QTcpSocket>
#include <QTimer>

#include "core/logging.h"

namespace {

// What pyatv says it is: receivers expect a recent enough AirPlay sender.
const char kUserAgent[] = "AirPlay/550.10";

QString Describe(const AirPlayRtspConnection::Headers& headers) {
  QStringList parts;
  for (const auto& h : headers) {
    parts << QString("%1: %2").arg(QString(h.first), QString(h.second));
  }
  return parts.join(", ");
}

}  // namespace

QByteArray AirPlayRtspConnection::Response::Header(
    const QByteArray& name) const {
  for (const auto& h : headers) {
    if (h.first.compare(name, Qt::CaseInsensitive) == 0) return h.second;
  }
  return QByteArray();
}

AirPlayRtspConnection::AirPlayRtspConnection(QObject* parent)
    : QObject(parent),
      socket_(nullptr),
      port_(0),
      connect_timeout_(new QTimer(this)),
      timeout_(new QTimer(this)),
      request_timeout_msec_(kTimeoutMsec),
      in_flight_(false),
      cseq_(0),
      session_id_(QRandomGenerator::global()->generate()),
      dacp_id_(QByteArray::number(QRandomGenerator::global()->generate64(), 16)
                   .toUpper()),
      active_remote_(QRandomGenerator::global()->generate()) {
  connect_timeout_->setSingleShot(true);
  connect_timeout_->setInterval(kConnectTimeoutMsec);
  connect(connect_timeout_, SIGNAL(timeout()), SLOT(ConnectTimedOut()));
  timeout_->setSingleShot(true);
  connect(timeout_, SIGNAL(timeout()), SLOT(TimedOut()));
  NewSocket();
}

void AirPlayRtspConnection::NewSocket() {
  socket_ = new QTcpSocket(this);
  connect(socket_, SIGNAL(connected()), connect_timeout_, SLOT(stop()));
  connect(socket_, SIGNAL(connected()), SIGNAL(Connected()));
  connect(socket_, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
          SLOT(SocketError()));
  connect(socket_, SIGNAL(readyRead()), SLOT(ReadyRead()));
}

AirPlayRtspConnection::~AirPlayRtspConnection() {}

void AirPlayRtspConnection::Connect(const QHostAddress& host, quint16 port) {
  host_ = host;
  port_ = port;
  connect_timeout_->start();
  socket_->connectToHost(host, port);
  socket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
}

void AirPlayRtspConnection::Close() {
  connect_timeout_->stop();
  timeout_->stop();
  queue_.clear();
  in_flight_ = false;
  buffer_.clear();
  cipher_.reset();
  if (socket_->state() != QAbstractSocket::ConnectedState) {
    socket_->abort();
    return;
  }
  // Whatever was written, such as a TEARDOWN, still goes, and the next
  // connection gets a socket of its own.
  QTcpSocket* old = socket_;
  old->disconnect(this);
  connect(old, &QAbstractSocket::disconnected, old, &QObject::deleteLater);
  old->disconnectFromHost();
  NewSocket();
}

void AirPlayRtspConnection::EnableEncryption(const QByteArray& out_key,
                                             const QByteArray& in_key) {
  cipher_.reset(new airplay::FrameCipher(out_key, in_key));
}

QHostAddress AirPlayRtspConnection::local_address() const {
  return socket_->localAddress();
}

void AirPlayRtspConnection::set_timeouts(int connect_msec, int request_msec) {
  connect_timeout_->setInterval(connect_msec);
  request_timeout_msec_ = request_msec;
}

QByteArray AirPlayRtspConnection::uri() const {
  return QString("rtsp://%1/%2")
      .arg(local_address().toString())
      .arg(session_id_)
      .toUtf8();
}

void AirPlayRtspConnection::Request(
    const QByteArray& method, const QByteArray& uri, const Headers& headers,
    const QByteArray& content_type, const QByteArray& body,
    const QByteArray& protocol, Callback callback, int timeout_msec) {
  Pending pending;
  pending.method = method;
  pending.uri = uri.isEmpty() ? this->uri() : uri;
  pending.headers = headers;
  pending.content_type = content_type;
  pending.body = body;
  pending.protocol = protocol;
  pending.callback = callback;
  pending.timeout_msec =
      timeout_msec < 0 ? request_timeout_msec_ : timeout_msec;
  queue_.enqueue(pending);
  SendNext();
}

void AirPlayRtspConnection::Rtsp(const QByteArray& method,
                                 const Headers& headers,
                                 const QByteArray& content_type,
                                 const QByteArray& body, Callback callback,
                                 int timeout_msec) {
  Request(method, QByteArray(), headers, content_type, body, "RTSP/1.0",
          callback, timeout_msec);
}

void AirPlayRtspConnection::Post(const QByteArray& path, const Headers& headers,
                                 const QByteArray& body, Callback callback,
                                 int timeout_msec) {
  Request("POST", path, headers, "application/octet-stream", body, "HTTP/1.1",
          callback, timeout_msec);
}

void AirPlayRtspConnection::SendNext() {
  if (in_flight_ || queue_.isEmpty()) return;
  in_flight_ = true;
  const Pending& pending = queue_.head();

  Headers all = {
      {"CSeq", QByteArray::number(cseq_++)},
      {"User-Agent", kUserAgent},
      {"DACP-ID", dacp_id_},
      {"Active-Remote", QByteArray::number(active_remote_)},
      {"Client-Instance", dacp_id_},
  };
  if (!client_name_.isEmpty()) {
    all.append({"X-Apple-Client-Name", client_name_});
  }
  all += pending.headers;
  if (!pending.content_type.isEmpty()) {
    all.append({"Content-Type", pending.content_type});
  }
  if (!pending.body.isEmpty()) {
    all.append({"Content-Length", QByteArray::number(pending.body.size())});
  }

  const QByteArray head =
      pending.method + " " + pending.uri + " " + pending.protocol;
  QByteArray message = head + "\r\n";
  for (const auto& h : all) message += h.first + ": " + h.second + "\r\n";
  message += "\r\n" + pending.body;
  qLog(Debug) << ">>" << head << Describe(all);

  // Encrypted when sent rather than when queued, since pairing turns
  // encryption on between requests.
  socket_->write(cipher_ ? cipher_->Encrypt(message) : message);
  timeout_->start(pending.timeout_msec);
}

void AirPlayRtspConnection::ReadyRead() {
  const QByteArray data = socket_->readAll();
  if (cipher_) {
    if (!cipher_->Decrypt(data, &buffer_)) {
      Close();
      emit Failed(tr("Couldn't decrypt what the AirPlay receiver sent"));
      return;
    }
  } else {
    buffer_ += data;
  }

  Response response;
  while (in_flight_ && ParseResponse(&response)) {
    timeout_->stop();
    qLog(Debug) << "<<" << response.code << Describe(response.headers)
                << response.body.size() << "byte body";
    in_flight_ = false;
    const Pending pending = queue_.dequeue();
    if (pending.callback) pending.callback(response);
    SendNext();
  }
}

bool AirPlayRtspConnection::ParseResponse(Response* response) {
  const int header_end = buffer_.indexOf("\r\n\r\n");
  if (header_end < 0) return false;
  const QList<QByteArray> lines = buffer_.left(header_end).split('\n');

  Response r;
  r.code = lines.value(0).trimmed().split(' ').value(1).toInt();
  for (int i = 1; i < lines.size(); ++i) {
    const int colon = lines[i].indexOf(':');
    if (colon < 0) continue;
    r.headers.append(
        {lines[i].left(colon).trimmed(), lines[i].mid(colon + 1).trimmed()});
  }
  const int length = r.Header("Content-Length").toInt();
  const int start = header_end + 4;
  if (buffer_.size() < start + length) return false;
  r.body = buffer_.mid(start, length);
  buffer_.remove(0, start + length);
  *response = r;
  return true;
}

void AirPlayRtspConnection::SocketError() {
  const QString error = socket_->errorString();
  Close();
  emit Failed(error);
}

// A receiver that's asleep may still answer pings through a sleep proxy, but
// never accepts the connection.
void AirPlayRtspConnection::ConnectTimedOut() {
  Close();
  emit Failed(tr("Couldn't connect to the AirPlay receiver at %1:%2 in %3 s. "
                 "Is it awake, with AirPlay on?")
                  .arg(host_.toString())
                  .arg(port_)
                  .arg(connect_timeout_->interval() / 1000.0));
}

void AirPlayRtspConnection::TimedOut() {
  const int msec = timeout_->interval();
  Close();
  emit Failed(
      tr("The AirPlay receiver didn't respond in %1 s").arg(msec / 1000.0));
}
