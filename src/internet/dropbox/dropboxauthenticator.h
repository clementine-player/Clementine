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

#ifndef INTERNET_DROPBOXAUTHENTICATOR_H_
#define INTERNET_DROPBOXAUTHENTICATOR_H_

#include <QObject>
#include <QTcpServer>

class LocalRedirectServer;
class NetworkAccessManager;
class QNetworkReply;

class DropboxAuthenticator : public QObject {
  Q_OBJECT

 public:
  explicit DropboxAuthenticator(QObject* parent = nullptr);
  void StartAuthorisation();

  const QString& access_token() const { return access_token_; }
  const QString& access_token_secret() const { return access_token_secret_; }
  const QString& uid() const { return uid_; }
  const QString& name() const { return name_; }

  static QByteArray GenerateAuthorisationHeader(const QString& token,
                                                const QString& secret);

 signals:
  void Finished();

 private slots:
  void RequestTokenFinished(QNetworkReply* reply);
  void RedirectArrived(LocalRedirectServer* server);
  void RequestAccessTokenFinished(QNetworkReply* reply);
  void RequestAccountInformationFinished(QNetworkReply* reply);

 private:
  void Authorise();
  void RequestAccessToken();
  QByteArray GenerateAuthorisationHeader();
  void RequestAccountInformation();

 private:
  NetworkAccessManager* network_;
  QTcpServer server_;

  // Temporary access token used for first authentication flow.
  QString token_;
  QString secret_;

  // Permanent OAuth access tokens.
  QString access_token_;
  QString access_token_secret_;

  // User's Dropbox uid & name.
  QString uid_;
  QString name_;
};

#endif  // INTERNET_DROPBOXAUTHENTICATOR_H_
