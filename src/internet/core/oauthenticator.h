/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#ifndef INTERNET_CORE_OAUTHENTICATOR_H_
#define INTERNET_CORE_OAUTHENTICATOR_H_

#include <QDateTime>
#include <QObject>

#include "core/network.h"

class LocalRedirectServer;
class QTcpSocket;

class OAuthenticator : public QObject {
  Q_OBJECT

 public:
  enum class RedirectStyle {
    // Redirect to localhost immediately.
    LOCALHOST = 0,
    // Redirect via data.clementine-player.org for when localhost is
    // unsupported (eg. Skydrive).
    REMOTE = 1,
    // Same as REMOTE, but pass the 'port' parameter of the redirect URL as
    // 'state' parameter of the URL, for services which allow only redirect URL
    // without parameters (e.g. SoundCloud). "state" parameter will be added to
    // the redirect URL by the service itself.
    REMOTE_WITH_STATE = 2
  };

  OAuthenticator(const QString& client_id, const QString& client_secret,
                 RedirectStyle redirect, QObject* parent = nullptr);
  void StartAuthorisation(const QString& oauth_endpoint,
                          const QString& token_endpoint, const QString& scope);
  void RefreshAuthorisation(const QString& token_endpoint,
                            const QString& refresh_token);

  // Token to use now.
  const QString& access_token() const { return access_token_; }

  // Token to use to get a new access token when it expires.
  const QString& refresh_token() const { return refresh_token_; }

  const QDateTime& expiry_time() const { return expiry_time_; }

 signals:
  void Finished();

 private slots:
  void RedirectArrived(LocalRedirectServer* server, QUrl url);
  void FetchAccessTokenFinished(QNetworkReply* reply);
  void RefreshAccessTokenFinished(QNetworkReply* reply);
  void SslErrors(const QList<QSslError>& errors);

 private:
  static const char* kRemoteURL;

  QByteArray ParseHttpRequest(const QByteArray& request) const;
  void RequestAccessToken(const QByteArray& code, const QUrl& url);
  void SetExpiryTime(int expires_in_seconds);

  NetworkAccessManager network_;

  QString access_token_;
  QString refresh_token_;
  QDateTime expiry_time_;

  const QString client_id_;
  const QString client_secret_;
  QUrl token_endpoint_;
  RedirectStyle redirect_style_;
};

#endif  // INTERNET_CORE_OAUTHENTICATOR_H_
