/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_GOOGLEDRIVE_GOOGLEDRIVECLIENT_H_
#define INTERNET_GOOGLEDRIVE_GOOGLEDRIVECLIENT_H_

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

class OAuthenticator;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

namespace google_drive {

class Client;

// Holds the metadata for a file on Google Drive.
class File {
 public:
  explicit File(const QVariantMap& data = QVariantMap()) : data_(data) {}

  static const char* kFolderMimeType;

  QString id() const { return data_["id"].toString(); }
  QString etag() const { return data_["etag"].toString(); }
  QString title() const { return data_["title"].toString(); }
  QString mime_type() const { return data_["mimeType"].toString(); }
  QString description() const { return data_["description"].toString(); }
  qint64 size() const { return data_["fileSize"].toUInt(); }
  QUrl download_url() const { return data_["downloadUrl"].toUrl(); }
  QUrl alternate_link() const { return data_["alternateLink"].toUrl(); }

  QDateTime modified_date() const {
    return QDateTime::fromString(data_["modifiedDate"].toString(), Qt::ISODate);
  }

  QDateTime created_date() const {
    return QDateTime::fromString(data_["createdDate"].toString(), Qt::ISODate);
  }

  bool is_folder() const { return mime_type() == kFolderMimeType; }
  QStringList parent_ids() const;

  bool has_label(const QString& name) const {
    return data_["labels"].toMap()[name].toBool();
  }

  bool is_starred() const { return has_label("starred"); }
  bool is_hidden() const { return has_label("hidden"); }
  bool is_trashed() const { return has_label("trashed"); }
  bool is_restricted() const { return has_label("restricted"); }
  bool is_viewed() const { return has_label("viewed"); }

 private:
  QVariantMap data_;
};

typedef QList<File> FileList;

class ConnectResponse : public QObject {
  Q_OBJECT
  friend class Client;

 public:
  const QString& refresh_token() const { return refresh_token_; }
  const QString& user_email() const { return user_email_; }

 signals:
  void Finished();

 private:
  explicit ConnectResponse(QObject* parent);
  QString refresh_token_;
  QString user_email_;
};

class GetFileResponse : public QObject {
  Q_OBJECT
  friend class Client;

 public:
  const QString& file_id() const { return file_id_; }
  const File& file() const { return file_; }
  bool had_error() { return had_error_; }

 signals:
  void Finished();

 private:
  GetFileResponse(const QString& file_id, QObject* parent);
  QString file_id_;
  File file_;
  bool had_error_;
};

class ListChangesResponse : public QObject {
  Q_OBJECT
  friend class Client;

 public:
  const QString& cursor() const { return cursor_; }
  const QString& next_cursor() const { return next_cursor_; }

 signals:
  void FilesFound(const QList<google_drive::File>& files);
  void FilesDeleted(const QList<QUrl>& files);
  void Finished();

 private:
  ListChangesResponse(const QString& cursor, QObject* parent);
  QString cursor_;
  QString next_cursor_;
};

class Client : public QObject {
  Q_OBJECT

 public:
  explicit Client(QObject* parent = nullptr);

  bool is_authenticated() const;
  const QString& access_token() const { return access_token_; }

  void ForgetCredentials();

  ConnectResponse* Connect(const QString& refresh_token = QString());
  GetFileResponse* GetFile(const QString& file_id);
  ListChangesResponse* ListChanges(const QString& cursor);

  QByteArray GetAuthHeader() const;

 signals:
  void Authenticated();

 private slots:
  void ConnectFinished(ConnectResponse* response, OAuthenticator* oauth);
  void FetchUserInfoFinished(ConnectResponse* response, QNetworkReply* reply);
  void GetFileFinished(GetFileResponse* response, QNetworkReply* reply);
  void ListChangesFinished(ListChangesResponse* response, QNetworkReply* reply);

 private:
  void AddAuthorizationHeader(QNetworkRequest* request) const;
  void MakeListChangesRequest(ListChangesResponse* response,
                              const QString& page_token = QString());

 private:
  QNetworkAccessManager* network_;

  QString access_token_;
  QDateTime expiry_time_;
};

}  // namespace google_drive

#endif  // INTERNET_GOOGLEDRIVE_GOOGLEDRIVECLIENT_H_
