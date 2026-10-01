/* This file is part of Clementine.
   Copyright 2012-2013, John Maguire <john.maguire@gmail.com>
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

#ifndef INTERNET_DROPBOX_DROPBOXSERVICE_H_
#define INTERNET_DROPBOX_DROPBOXSERVICE_H_

#include <QDateTime>

#include "core/tagreaderclient.h"
#include "internet/core/cloudfileservice.h"

class NetworkAccessManager;
class OAuthenticator;
class QNetworkReply;

class DropboxService : public CloudFileService {
  Q_OBJECT

 public:
  DropboxService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;

  // Opens the browser to log in to Dropbox.
  void Login();
  void ForgetCredentials();

  QUrl GetStreamingUrlFromSongId(const QUrl& url);

 signals:
  void Connected();

 public slots:
  void Connect();

 private slots:
  void RequestFileListFinished(QNetworkReply* reply);
  void FetchContentUrlFinished(QNetworkReply* reply, const QVariantMap& file);
  void LongPollFinished(QNetworkReply* reply);
  void LongPollDelta();

 private:
  QString refresh_token() const;
  bool is_authenticated() const;
  // Gets a new access token with the refresh token. It's ready when the
  // OAuthenticator returned emits Finished().
  OAuthenticator* RefreshAccessToken();
  void AccessTokenFinished(OAuthenticator* oauth);
  // Blocks until the access token is current, if it can be.
  void EnsureConnected();

  void RequestFileList();
  QByteArray GenerateAuthorisationHeader();
  QNetworkReply* FetchContentUrl(const QUrl& url);

 private:
  QString access_token_;
  // When access_token_ expires, or invalid for a long-lived token.
  QDateTime expiry_time_;

  NetworkAccessManager* network_;
};

#endif  // INTERNET_DROPBOX_DROPBOXSERVICE_H_
