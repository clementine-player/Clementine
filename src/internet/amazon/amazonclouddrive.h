/* This file is part of Clementine.
   Copyright 2015, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_AMAZON_AMAZON_CLOUD_DRIVE_H_
#define INTERNET_AMAZON_AMAZON_CLOUD_DRIVE_H_

#include "internet/core/cloudfileservice.h"

#include <QDateTime>
#include <QString>
#include <QUrl>

class NetworkAccessManager;
class OAuthenticator;
class QNetworkReply;
class QNetworkRequest;

class AmazonCloudDrive : public CloudFileService {
  Q_OBJECT
 public:
  AmazonCloudDrive(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;

  QUrl GetStreamingUrlFromSongId(const QUrl& url);

  void ForgetCredentials();

 signals:
  void Connected();

 public slots:
  void Connect();

 private:
  void FetchEndpoint();
  void RequestChanges(const QString& checkpoint);
  void AddAuthorizationHeader(QNetworkRequest* request);
  void EnsureConnected();

 private slots:
  void ConnectFinished(OAuthenticator*);
  void FetchEndpointFinished(QNetworkReply*);
  void RequestChangesFinished(QNetworkReply*);

 private:
  NetworkAccessManager* network_;
  QString access_token_;
  QDateTime expiry_time_;
  QString content_url_;
  QString metadata_url_;
};

#endif  // INTERNET_AMAZON_AMAZON_CLOUD_DRIVE_H_
