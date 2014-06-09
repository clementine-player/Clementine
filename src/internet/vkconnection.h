/* This file is part of Clementine.
   Copyright 2014, Vlad Maltsev <shedwardx@gmail.com>

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

#ifndef VKCONNECTION_H
#define VKCONNECTION_H

#include "vreen/client.h"
#include "vreen/connection.h"

class LocalRedirectServer;

class VkConnection : public Vreen::Connection {
  Q_OBJECT
  Q_ENUMS(DisplayType)
  Q_FLAGS(Scopes)

public:
  enum DisplayType {
    Page,
    Popup,
    Touch,
    Wap
  };
  enum Scope {
    Notify        = 0x1,
    Friends       = 0x2,
    Photos        = 0x4,
    Audio         = 0x8,
    Video         = 0x10,
    Docs          = 0x20,
    Notes         = 0x40,
    Pages         = 0x80,
    Status        = 0x100,
    Offers        = 0x200,
    Questions     = 0x400,
    Wall          = 0x800,
    Groups        = 0x1000,
    Messages      = 0x2000,
    Notifications = 0x4000,
    Stats         = 0x8000,
    Ads           = 0x10000,
    Offline       = 0x20000
  };
  Q_DECLARE_FLAGS(Scopes, Scope)

  explicit VkConnection(QObject* parent = 0);
  ~VkConnection();

  void connectToHost(const QString& login, const QString& password);
  void disconnectFromHost();
  Vreen::Client::State connectionState() const { return state_; }
  int uid() const { return uid_; }
  void clear();
  bool hasAccount();

protected:
  QNetworkRequest makeRequest(const QString& method,
                              const QVariantMap& args = QVariantMap());
  void decorateRequest(QNetworkRequest& request);

private slots:
  void codeRecived(LocalRedirectServer* server, QUrl redirect_uri);
  void accessTokenRecived(QNetworkReply* reply);

private:
  void requestAccessToken();
  void setConnectionState(Vreen::Client::State state);
  void saveToken();
  void loadToken();
  QNetworkAccessManager network_;
  Vreen::Client::State state_;
  QByteArray code_;
  QByteArray access_token_;
  time_t expires_in_;
  int uid_;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VkConnection::Scopes)

#endif  // VKCONNECTION_H
