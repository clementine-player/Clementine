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

// Holds the metadata for a file on Google Drive (Drive API v3 File
// resource - see Client::GetFile/ListChanges, which both request exactly
// the fields read below via a "fields=" parameter).
class File {
 public:
  explicit File(const QVariantMap& data = QVariantMap()) : data_(data) {}

  QString id() const { return data_["id"].toString(); }
  // v3 has no generic per-version etag; "version" is a monotonically
  // increasing int64 (encoded as a string, per Google API convention) that
  // serves the same change-detection purpose.
  QString etag() const { return data_["version"].toString(); }
  QString title() const { return data_["name"].toString(); }
  QString mime_type() const { return data_["mimeType"].toString(); }
  QString description() const { return data_["description"].toString(); }
  qint64 size() const { return data_["size"].toString().toLongLong(); }
  // v3 dropped the v2 "downloadUrl" field - content is fetched from the
  // same file resource endpoint with alt=media, still requiring the same
  // Authorization header used for metadata requests (see
  // GoogleDriveService::FilesFound / GetStreamingUrlFromSongId, which
  // already pass that header alongside this URL).
  QUrl download_url() const;

  QDateTime modified_date() const {
    return QDateTime::fromString(data_["modifiedTime"].toString(), Qt::ISODate);
  }

  QDateTime created_date() const {
    return QDateTime::fromString(data_["createdTime"].toString(), Qt::ISODate);
  }

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
  // Only ever populated by Client::AuthorizeAndPick - empty for a plain
  // Client::Connect() (silent refresh, or a login with no picking).
  const QStringList& picked_file_ids() const { return picked_file_ids_; }

 signals:
  void Finished();

 private:
  explicit ConnectResponse(QObject* parent);
  QString refresh_token_;
  QString user_email_;
  QStringList picked_file_ids_;
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
  // The second argument allows for specifying a custom network access
  // manager. It is used in tests. Ownership of network is not transferred.
  explicit Client(QObject* parent = nullptr,
                  QNetworkAccessManager* network = nullptr);

  bool is_authenticated() const;
  const QString& access_token() const { return access_token_; }

  void ForgetCredentials();

  // Silent reconnect: requires an existing refresh_token and never opens a
  // browser. Used for background/session reconnects only - grants nothing
  // new, since drive.file access can only ever be extended via
  // AuthorizeAndPick.
  ConnectResponse* Connect(const QString& refresh_token);
  // Performs Google's combined OAuth-consent + Picker "One Pick" flow
  // required to get drive.file access to specific files on a desktop app
  // (see
  // https://developers.google.com/workspace/drive/picker/guides/desktop-mobile-picker).
  // Always opens the system browser, even if already connected - a plain
  // login without picking anything grants access to nothing at all under
  // drive.file scope. Returns a fresh refresh token plus whatever the user
  // picked, via ConnectResponse::picked_file_ids().
  ConnectResponse* AuthorizeAndPick(const QStringList& mime_types);
  // |resource_key| is required to access some Drive items (mostly ones
  // shared via link before Sept. 2021) even with a valid access grant.
  GetFileResponse* GetFile(const QString& file_id,
                          const QString& resource_key = QString());
  ListChangesResponse* ListChanges(const QString& cursor);

  QByteArray GetAuthHeader() const;

 signals:
  void Authenticated();

 private slots:
  void ConnectFinished(ConnectResponse* response, OAuthenticator* oauth);
  void FetchUserInfoFinished(ConnectResponse* response, QNetworkReply* reply);
  void GetFileFinished(GetFileResponse* response, QNetworkReply* reply);
  void ListChangesFinished(ListChangesResponse* response, QNetworkReply* reply);
  void StartPageTokenFinished(ListChangesResponse* response,
                              QNetworkReply* reply);

 private:
  void AddAuthorizationHeader(QNetworkRequest* request) const;
  void AddResourceKeyHeader(QNetworkRequest* request, const QString& file_id,
                            const QString& resource_key) const;
  // v3's Changes API has no "list everything since account creation"
  // concept the way v2's empty startChangeId did - a cursor must first be
  // established via changes/startPageToken. That's fine for us: picked
  // files are already indexed directly (see GoogleDriveService::
  // AddPickedItemFinished), so Changes only ever needs to track things
  // going forward from here.
  void RequestStartPageToken(ListChangesResponse* response);
  void MakeListChangesRequest(ListChangesResponse* response,
                              const QString& page_token);

 private:
  QNetworkAccessManager* network_;

  QString access_token_;
  QDateTime expiry_time_;
};

}  // namespace google_drive

#endif  // INTERNET_GOOGLEDRIVE_GOOGLEDRIVECLIENT_H_
