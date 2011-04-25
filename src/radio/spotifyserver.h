/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef SPOTIFYSERVER_H
#define SPOTIFYSERVER_H

#include <QObject>

class RequestMessage;

class QTcpServer;
class QTcpSocket;

class SpotifyServer : public QObject {
  Q_OBJECT

public:
  SpotifyServer(QObject* parent = 0);

  void Init();
  void Login(const QString& username, const QString& password);

  int server_port() const;

signals:
  void ClientConnected();
  void LoginCompleted(bool success);

private slots:
  void NewConnection();
  void ProtocolSocketReadyRead();

private:
  void SendMessage(const RequestMessage& message);

  QTcpServer* server_;
  QTcpSocket* protocol_socket_;
};

#endif // SPOTIFYSERVER_H
