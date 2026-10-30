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

#include "rtspconnection.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

#include "logging.h"

namespace airplay {

namespace {

const char kUserAgent[] = "AirPlay/550.10";
const char kDigestUsername[] = "iTunes";
const int kConnectTimeoutMsec = 5000;

QString Describe(const Headers& headers) {
  QStringList parts;
  for (const auto& h : headers) {
    parts << QString("%1: %2").arg(QString(h.first), QString(h.second));
  }
  return parts.join(", ");
}

}  // namespace

QByteArray RtspConnection::Response::Header(const QByteArray& name) const {
  for (const auto& h : headers) {
    if (h.first.compare(name, Qt::CaseInsensitive) == 0) return h.second;
  }
  return QByteArray();
}

RtspConnection::RtspConnection(QObject* parent)
    : QObject(parent),
      socket_(new QTcpSocket(this)),
      connect_timeout_(new QTimer(this)),
      timeout_(new QTimer(this)),
      session_id_(QRandomGenerator::global()->generate()),
      dacp_id_(QByteArray::number(QRandomGenerator::global()->generate64(), 16)
                   .toUpper()),
      active_remote_(QRandomGenerator::global()->generate()) {
  connect_timeout_->setSingleShot(true);
  connect_timeout_->setInterval(kConnectTimeoutMsec);
  connect(connect_timeout_, SIGNAL(timeout()), SLOT(ConnectTimedOut()));
  timeout_->setSingleShot(true);
  timeout_->setInterval(kTimeoutMsec);
  connect(timeout_, SIGNAL(timeout()), SLOT(TimedOut()));
  connect(socket_, SIGNAL(connected()), connect_timeout_, SLOT(stop()));
  connect(socket_, SIGNAL(connected()), SIGNAL(Connected()));
  connect(socket_, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
          SLOT(SocketError()));
  connect(socket_, SIGNAL(readyRead()), SLOT(ReadyRead()));
}

void RtspConnection::Connect(const QHostAddress& host, quint16 port) {
  host_ = host;
  port_ = port;
  connect_timeout_->start();
  socket_->connectToHost(host, port);
  socket_->setSocketOption(QAbstractSocket::LowDelayOption, 1);
}

void RtspConnection::EnableEncryption(const QByteArray& out_key,
                                      const QByteArray& in_key) {
  cipher_.reset(new FrameCipher(out_key, in_key));
}

QByteArray RtspConnection::uri() const {
  return QString("rtsp://%1/%2")
      .arg(local_address().toString())
      .arg(session_id_)
      .toUtf8();
}

void RtspConnection::Request(const QByteArray& method, const QByteArray& uri,
                             const Headers& headers,
                             const QByteArray& content_type,
                             const QByteArray& body, const QByteArray& protocol,
                             Callback callback, int timeout_msec) {
  Pending pending;
  pending.method = method;
  pending.uri = uri.isEmpty() ? this->uri() : uri;
  pending.headers = headers;
  pending.content_type = content_type;
  pending.body = body;
  pending.protocol = protocol;
  pending.callback = callback;
  pending.timeout_msec = timeout_msec;
  queue_.enqueue(pending);
  SendNext();
}

// HTTP Digest (RFC 2617 without qop), as AirPlay 1 receivers with a password
// ask for.
QByteArray RtspConnection::DigestAuthorization(const QByteArray& method,
                                               const QByteArray& uri) const {
  auto md5 = [](const QByteArray& data) {
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
  };
  const QByteArray ha1 =
      md5(QByteArray(kDigestUsername) + ":" + digest_realm_ + ":" + password_);
  const QByteArray ha2 = md5(method + ":" + uri);
  return QByteArray("Digest username=\"") + kDigestUsername + "\", realm=\"" +
         digest_realm_ + "\", nonce=\"" + digest_nonce_ + "\", uri=\"" + uri +
         "\", response=\"" + md5(ha1 + ":" + digest_nonce_ + ":" + ha2) + "\"";
}

void RtspConnection::Rtsp(const QByteArray& method, const Headers& headers,
                          const QByteArray& content_type,
                          const QByteArray& body, Callback callback,
                          int timeout_msec) {
  Request(method, QByteArray(), headers, content_type, body, "RTSP/1.0",
          callback, timeout_msec);
}

void RtspConnection::Post(const QByteArray& path, const Headers& headers,
                          const QByteArray& body, Callback callback,
                          int timeout_msec) {
  Request("POST", path, headers, "application/octet-stream", body, "HTTP/1.1",
          callback, timeout_msec);
}

void RtspConnection::SendNext() {
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
  if (!digest_nonce_.isEmpty()) {
    all.append(
        {"Authorization", DigestAuthorization(pending.method, pending.uri)});
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
  QStringList logged = {QString(head)};
  for (const auto& h : all) {
    message += h.first + ": " + h.second + "\r\n";
    // The digest is derived from the password, so it isn't logged.
    logged << QString(h.first + ": " +
                      (h.first == "Authorization" ? "(digest)" : h.second));
  }
  message += "\r\n" + pending.body;
  qCDebug(airplay_log) << ">>" << qUtf8Printable(logged.join(" | "));

  // Encrypted when sent rather than when queued, since pairing turns
  // encryption on between requests.
  socket_->write(cipher_ ? cipher_->Encrypt(message) : message);
  timeout_->start(pending.timeout_msec);
}

void RtspConnection::ReadyRead() {
  const QByteArray data = socket_->readAll();
  if (cipher_) {
    if (!cipher_->Decrypt(data, &buffer_)) {
      emit Failed("couldn't decrypt a frame from the receiver");
      socket_->abort();
      return;
    }
  } else {
    buffer_ += data;
  }

  Response response;
  while (in_flight_ && ParseResponse(&response)) {
    timeout_->stop();
    qCDebug(airplay_log) << "<<" << response.code
                         << qUtf8Printable(Describe(response.headers))
                         << QString("(%1 byte body)").arg(response.body.size());
    in_flight_ = false;
    if (response.code == 401 && Authenticate(response)) {
      SendNext();  // The same request again, with a digest.
      continue;
    }
    const Pending pending = queue_.dequeue();
    if (pending.callback) pending.callback(response);
    SendNext();
  }
}

bool RtspConnection::Authenticate(const Response& response) {
  Pending* pending = &queue_.head();
  const QByteArray challenge = response.Header("WWW-Authenticate");
  if (password_.isEmpty() || pending->authenticated ||
      !challenge.startsWith("Digest ")) {
    return false;
  }
  for (const QByteArray& part : challenge.mid(7).split(',')) {
    const int eq = part.indexOf('=');
    if (eq < 0) continue;
    const QByteArray key = part.left(eq).trimmed();
    QByteArray value = part.mid(eq + 1).trimmed();
    if (value.startsWith('"') && value.endsWith('"')) {
      value = value.mid(1, value.size() - 2);
    }
    if (key == "realm") digest_realm_ = value;
    if (key == "nonce") digest_nonce_ = value;
  }
  pending->authenticated = true;
  qCDebug(airplay_log) << "Answering a Digest challenge for realm"
                       << digest_realm_;
  return !digest_nonce_.isEmpty();
}

bool RtspConnection::ParseResponse(Response* response) {
  const int header_end = buffer_.indexOf("\r\n\r\n");
  if (header_end < 0) return false;
  const QList<QByteArray> lines = buffer_.left(header_end).split('\n');

  Response r;
  const QList<QByteArray> status = lines.value(0).trimmed().split(' ');
  r.code = status.value(1).toInt();
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

void RtspConnection::SocketError() {
  connect_timeout_->stop();
  timeout_->stop();
  emit Failed(socket_->errorString());
}

// A receiver that's asleep may still answer pings through a sleep proxy, but
// never accepts the connection.
void RtspConnection::ConnectTimedOut() {
  socket_->abort();
  emit Failed(QString("couldn't connect to %1:%2 in %3 s (is it awake, with "
                      "AirPlay Receiver on?)")
                  .arg(host_.toString())
                  .arg(port_)
                  .arg(kConnectTimeoutMsec / 1000));
}

void RtspConnection::TimedOut() {
  emit Failed(QString("no response from the receiver in %1 s")
                  .arg(timeout_->interval() / 1000));
}

}  // namespace airplay
