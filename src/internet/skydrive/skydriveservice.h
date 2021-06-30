/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef INTERNET_SKYDRIVE_SKYDRIVESERVICE_H_
#define INTERNET_SKYDRIVE_SKYDRIVESERVICE_H_

#include <QDateTime>

#include "internet/core/cloudfileservice.h"

class OAuthenticator;
class QNetworkRequest;
class QNetworkReply;

class SkydriveService : public CloudFileService {
  Q_OBJECT

 public:
  SkydriveService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const override;
  QUrl GetStreamingUrlFromSongId(const QString& song_id);

  QString GetScheme() const { return "onedrive"; }

 public slots:
  virtual void Connect() override;
  void ForgetCredentials();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);
  void FetchUserInfoFinished(QNetworkReply* reply);
  void ListFilesFinished(QNetworkReply* reply);

 signals:
  void Connected();

 private:
  friend class SkydriveUrlHandler;
  QByteArray GetAuthHeader() const;

 private:
  QString refresh_token() const;
  void AddAuthorizationHeader(QNetworkRequest* request);
  void FetchUserInfo();
  void ListFiles();
  void ListFiles(const QString& folder);
  void EnsureConnected();
  QUrl ItemUrl(const QString& id, const QString& path);
  void DoFullRescan() override;

  void PopulateContextMenu() override;
  void UpdateContextMenu() override;

  QString access_token_;
  QDateTime expiry_time_;

  QAction* full_rescan_action_;
};

#endif  // INTERNET_SKYDRIVE_SKYDRIVESERVICE_H_
