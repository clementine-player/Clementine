/* This file is part of Clementine.
   Copyright 2013-2014, John Maguire <john.maguire@gmail.com>
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

#ifndef INTERNET_BOX_BOXSERVICE_H_
#define INTERNET_BOX_BOXSERVICE_H_

#include "internet/core/cloudfileservice.h"

#include <QDateTime>

class OAuthenticator;
class QNetworkReply;
class QNetworkRequest;

class BoxService : public CloudFileService {
  Q_OBJECT

 public:
  BoxService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;
  QUrl GetStreamingUrlFromSongId(const QString& id);

 public slots:
  void Connect();
  void ForgetCredentials();

 signals:
  void Connected();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);
  void FetchUserInfoFinished(QNetworkReply* reply);
  void FetchFolderItemsFinished(QNetworkReply* reply, const int folder_id);
  void RedirectFollowed(QNetworkReply* reply, const Song& song,
                        const QString& mime_type);
  void InitialiseEventsFinished(QNetworkReply* reply);
  void FetchEventsFinished(QNetworkReply* reply);

 private:
  QString refresh_token() const;
  bool is_authenticated() const;
  void AddAuthorizationHeader(QNetworkRequest* request) const;
  void UpdateFiles();
  void FetchRecursiveFolderItems(const int folder_id, const int offset = 0);
  void UpdateFilesFromCursor(const QString& cursor);
  QNetworkReply* FetchContentUrlForFile(const QString& file_id);
  void InitialiseEventsCursor();
  void MaybeAddFileEntry(const QJsonObject& entry);
  void EnsureConnected();

  QString access_token_;
  QDateTime expiry_time_;
};

#endif  // INTERNET_BOX_BOXSERVICE_H_
