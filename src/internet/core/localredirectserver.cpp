/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "localredirectserver.h"

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QRegularExpression>
#include <QStyle>
#include <QTcpServer>
#include <QTcpSocket>

LocalRedirectServer::LocalRedirectServer(QObject* parent)
    : QObject(parent), server_(new QTcpServer(this)) {}

void LocalRedirectServer::Listen() {
  server_->listen(QHostAddress::LocalHost);
  // We have to calculate this and store it now as the server port is cleared
  // once we close the socket.
  url_.setScheme("http");
  url_.setHost("localhost");
  url_.setPort(server_->serverPort());
  url_.setPath("/");
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
}

void LocalRedirectServer::NewConnection() {
  // Accept only one redirect connection; guard against spurious null
  // sockets. Accumulate reads in buffer_ via a persistent connection rather
  // than re-arming one-shot closures with copied buffers.
  QTcpSocket* socket = server_->nextPendingConnection();
  server_->close();
  if (!socket) return;

  buffer_.clear();
  connect(socket, &QTcpSocket::readyRead, this,
          [this, socket]() { ReadyRead(socket); });
}

void LocalRedirectServer::ReadyRead(QTcpSocket* socket) {
  buffer_.append(socket->readAll());
  // Wait for the HTTP header terminator or peer close, then disconnect before
  // replying so trailing packets cannot re-emit Finished().
  if (buffer_.contains("\r\n\r\n") || buffer_.contains("\n\n") ||
      socket->state() == QAbstractSocket::UnconnectedState) {
    socket->disconnect(this);
    WriteTemplate(socket);
    socket->deleteLater();
    request_url_ = ParseUrlFromRequest(buffer_);
    buffer_.clear();
    emit Finished();
  }
}

void LocalRedirectServer::WriteTemplate(QTcpSocket* socket) const {
  QFile page_file(":oauthsuccess.html");
  (void)page_file.open(QIODevice::ReadOnly);
  QString page_data = QString::fromUtf8(page_file.readAll());

  static const QRegularExpression tr_regexp("tr\\(\"([^\"]+)\"\\)");
  int offset = 0;
  while (true) {
    const QRegularExpressionMatch match = tr_regexp.match(page_data, offset);
    if (!match.hasMatch()) {
      break;
    }

    page_data.replace(match.capturedStart(), match.capturedLength(),
                      tr(match.captured(1).toUtf8()));
    offset = match.capturedStart() + match.capturedLength();
  }

  QBuffer image_buffer;
  image_buffer.open(QIODevice::ReadWrite);
  QApplication::style()
      ->standardIcon(QStyle::SP_DialogOkButton)
      .pixmap(16)
      .toImage()
      .save(&image_buffer, "PNG");
  page_data.replace("@IMAGE_DATA@", image_buffer.data().toBase64());

  socket->write("HTTP/1.0 200 OK\r\n");
  socket->write("Content-type: text/html;charset=UTF-8\r\n");
  socket->write("\r\n\r\n");
  socket->write(page_data.toUtf8());
  socket->flush();
}

QUrl LocalRedirectServer::ParseUrlFromRequest(const QByteArray& request) const {
  // Guard against truncated or malformed requests missing a target path.
  const QList<QByteArray> lines = request.split('\r');
  if (lines.isEmpty()) return QUrl();
  const QList<QByteArray> parts = lines[0].split(' ');
  if (parts.size() < 2) return QUrl();
  const QByteArray& path = parts[1];
  // url() already ends with '/'; strip at most one leading '/' from path.
  const QString relative =
      QString::fromUtf8(path.startsWith('/') ? path.mid(1) : path);
  QUrl request_url(url().toString() + relative, QUrl::StrictMode);
  return request_url;
}
