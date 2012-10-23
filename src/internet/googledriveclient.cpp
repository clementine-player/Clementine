/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "googledriveclient.h"
#include "oauthenticator.h"
#include "core/closure.h"
#include "core/network.h"

#include <qjson/parser.h>

using namespace google_drive;

const char* File::kFolderMimeType = "application/vnd.google-apps.folder";

namespace {
  static const char* kGoogleDriveFiles = "https://www.googleapis.com/drive/v2/files";
  static const char* kGoogleDriveFile = "https://www.googleapis.com/drive/v2/files/%1";
}

QStringList File::parent_ids() const {
  QStringList ret;

  foreach (const QVariant& var, data_["parents"].toList()) {
    QVariantMap map(var.toMap());

    if (map["isRoot"].toBool()) {
      ret << QString();
    } else {
      ret << map["id"].toString();
    }
  }

  return ret;
}

ConnectResponse::ConnectResponse(QObject* parent)
  : QObject(parent)
{
}

ListFilesResponse::ListFilesResponse(const QString& query, QObject* parent)
  : QObject(parent),
    query_(query)
{
}

GetFileResponse::GetFileResponse(const QString& file_id, QObject* parent)
  : QObject(parent),
    file_id_(file_id)
{
}

Client::Client(QObject* parent)
  : QObject(parent),
    network_(new NetworkAccessManager(this))
{
}

ConnectResponse* Client::Connect(const QString& refresh_token) {
  ConnectResponse* ret = new ConnectResponse(this);
  OAuthenticator* oauth = new OAuthenticator(this);

  if (refresh_token.isEmpty()) {
    oauth->StartAuthorisation();
  } else {
    oauth->RefreshAuthorisation(refresh_token);
  }

  NewClosure(oauth, SIGNAL(Finished()),
             this, SLOT(ConnectFinished(ConnectResponse*,OAuthenticator*)),
             ret, oauth);
  return ret;
}

void Client::ConnectFinished(ConnectResponse* response, OAuthenticator* oauth) {
  oauth->deleteLater();
  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();
  response->refresh_token_ = oauth->refresh_token();
  response->user_email_ = oauth->user_email();
  emit response->Finished();

  emit Authenticated();
}

void Client::AddAuthorizationHeader(QNetworkRequest* request) const {
  request->setRawHeader(
      "Authorization", QString("Bearer %1").arg(access_token_).toUtf8());
}

ListFilesResponse* Client::ListFiles(const QString& query) {
  ListFilesResponse* ret = new ListFilesResponse(query, this);
  MakeListFilesRequest(ret);
  return ret;
}

void Client::MakeListFilesRequest(ListFilesResponse* response, const QString& page_token) {
  QUrl url = QUrl(kGoogleDriveFiles);

  if (!response->query_.isEmpty()) {
    url.addQueryItem("q", response->query_);
  }

  if (!page_token.isEmpty()) {
    url.addQueryItem("pageToken", page_token);
  }

  QNetworkRequest request = QNetworkRequest(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(ListFilesFinished(ListFilesResponse*, QNetworkReply*)),
             response, reply);
}

void Client::ListFilesFinished(ListFilesResponse* response, QNetworkReply* reply) {
  reply->deleteLater();

  // Parse the response
  QJson::Parser parser;
  bool ok = false;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Failed to request files from Google Drive";
    emit response->Finished();
    return;
  }

  // Emit the FilesFound signal for the files in the response.
  FileList files;
  foreach (const QVariant& v, result["items"].toList()) {
    files << File(v.toMap());
  }

  emit response->FilesFound(files);

  // Get the next page of results if there is one.
  if (result.contains("nextPageToken")) {
    MakeListFilesRequest(response, result["nextPageToken"].toString());
  } else {
    emit response->Finished();
  }
}

GetFileResponse* Client::GetFile(const QString& file_id) {
  GetFileResponse* ret = new GetFileResponse(file_id, this);

  QUrl url(QString(kGoogleDriveFile).arg(file_id));

  QNetworkRequest request = QNetworkRequest(url);
  AddAuthorizationHeader(&request);
  // Never cache these requests as we will get out of date download URLs.
  request.setAttribute(
      QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(GetFileFinished(GetFileResponse*,QNetworkReply*)),
             ret, reply);

  return ret;
}

void Client::GetFileFinished(GetFileResponse* response, QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;
  bool ok = false;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Failed to fetch file with ID" << response->file_id_;
    emit response->Finished();
    return;
  }

  response->file_ = File(result);
  emit response->Finished();
}

bool Client::is_authenticated() const {
  return !access_token_.isEmpty() &&
      QDateTime::currentDateTime().secsTo(expiry_time_) > 0;
}

void Client::ForgetCredentials() {
  access_token_ = QString();
  expiry_time_ = QDateTime();
}
