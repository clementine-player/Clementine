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

#include "internet/core/cloudfileservice.h"

#include "core/tagreaderclient.h"

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

  QUrl GetStreamingUrlFromSongId(const QUrl& url);

signals:
  void Connected();

 public slots:
  void Connect();
  void AuthenticationFinished(OAuthenticator* authenticator);

 private slots:
  void RequestFileListFinished(QNetworkReply* reply);
  void FetchContentUrlFinished(QNetworkReply* reply, const QVariantMap& file);
  void LongPollFinished(QNetworkReply* reply);
  void LongPollDelta();

 private:
  void RequestFileList();
  QByteArray GenerateAuthorisationHeader();
  QNetworkReply* FetchContentUrl(const QUrl& url);

 private:
  QString access_token_;

  NetworkAccessManager* network_;
};

#endif  // INTERNET_DROPBOX_DROPBOXSERVICE_H_
