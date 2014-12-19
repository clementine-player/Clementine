/* This file is part of Clementine.
   Copyright 2012, John Maguire <john.maguire@gmail.com>
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

#ifndef INTERNET_CORE_LOCALREDIRECTSERVER_H_
#define INTERNET_CORE_LOCALREDIRECTSERVER_H_

#include <QByteArray>
#include <QObject>
#include <QUrl>

class QTcpServer;
class QTcpSocket;

class LocalRedirectServer : public QObject {
  Q_OBJECT

 public:
  explicit LocalRedirectServer(QObject* parent = nullptr);

  // Causes the server to listen for _one_ request.
  void Listen();

  // Returns the HTTP URL of this server.
  const QUrl& url() const { return url_; }

  // Returns the URL requested by the OAuth redirect.
  const QUrl& request_url() const { return request_url_; }

 signals:
  void Finished();

 private slots:
  void NewConnection();
  void ReadyRead(QTcpSocket* socket, QByteArray buffer);

 private:
  void WriteTemplate(QTcpSocket* socket) const;
  QUrl ParseUrlFromRequest(const QByteArray& request) const;

 private:
  QTcpServer* server_;
  QUrl url_;
  QUrl request_url_;
};

#endif  // INTERNET_CORE_LOCALREDIRECTSERVER_H_
