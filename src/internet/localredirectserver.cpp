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
#include <QRegExp>
#include <QStyle>
#include <QTcpServer>
#include <QTcpSocket>

#include "core/closure.h"

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
  QTcpSocket* socket = server_->nextPendingConnection();
  server_->close();

  QByteArray buffer;
  NewClosure(socket, SIGNAL(readyRead()), this,
             SLOT(ReadyRead(QTcpSocket*, QByteArray)), socket, buffer);
}

void LocalRedirectServer::ReadyRead(QTcpSocket* socket, QByteArray buffer) {
  buffer.append(socket->readAll());
  if (socket->atEnd() || buffer.endsWith("\r\n\r\n")) {
    WriteTemplate(socket);
    socket->deleteLater();
    request_url_ = ParseUrlFromRequest(buffer);
    emit Finished();
  } else {
    NewClosure(socket, SIGNAL(readyRead()), this,
               SLOT(ReadyReady(QTcpSocket*, QByteArray)), socket, buffer);
  }
}

void LocalRedirectServer::WriteTemplate(QTcpSocket* socket) const {
  QFile page_file(":oauthsuccess.html");
  page_file.open(QIODevice::ReadOnly);
  QString page_data = QString::fromUtf8(page_file.readAll());

  QRegExp tr_regexp("tr\\(\"([^\"]+)\"\\)");
  int offset = 0;
  forever {
    offset = tr_regexp.indexIn(page_data, offset);
    if (offset == -1) {
      break;
    }

    page_data.replace(offset, tr_regexp.matchedLength(),
                      tr(tr_regexp.cap(1).toUtf8()));
    offset += tr_regexp.matchedLength();
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
  QList<QByteArray> lines = request.split('\r');
  const QByteArray& request_line = lines[0];
  QByteArray path = request_line.split(' ')[1];
  QUrl base_url = url();
  QUrl request_url(base_url.toString() + path.mid(1), QUrl::StrictMode);
  return request_url;
}
