/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012, 2014, David Sansome <me@davidsansome.com>
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

#include "googledriveclient.h"

#include <qjson/parser.h>

#include "oauthenticator.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

using namespace google_drive;

const char* File::kFolderMimeType = "application/vnd.google-apps.folder";

namespace {
static const char* kGoogleDriveFile =
    "https://www.googleapis.com/drive/v2/files/%1";
static const char* kGoogleDriveChanges =
    "https://www.googleapis.com/drive/v2/changes";
static const char* kGoogleOAuthUserInfoEndpoint =
    "https://www.googleapis.com/oauth2/v1/userinfo";

static const char* kOAuthEndpoint = "https://accounts.google.com/o/oauth2/auth";
static const char* kOAuthTokenEndpoint =
    "https://accounts.google.com/o/oauth2/token";
static const char* kOAuthScope =
    "https://www.googleapis.com/auth/drive.readonly "
    "https://www.googleapis.com/auth/userinfo.email";
static const char* kClientId = "679260893280.apps.googleusercontent.com";
static const char* kClientSecret = "l3cWb8efUZsrBI4wmY3uKl6i";
}  // namespace

QStringList File::parent_ids() const {
  QStringList ret;

  for (const QVariant& var : data_["parents"].toList()) {
    QVariantMap map(var.toMap());

    if (map["isRoot"].toBool()) {
      ret << QString();
    } else {
      ret << map["id"].toString();
    }
  }

  return ret;
}

ConnectResponse::ConnectResponse(QObject* parent) : QObject(parent) {}

GetFileResponse::GetFileResponse(const QString& file_id, QObject* parent)
    : QObject(parent), file_id_(file_id) {}

ListChangesResponse::ListChangesResponse(const QString& cursor, QObject* parent)
    : QObject(parent), cursor_(cursor) {}

Client::Client(QObject* parent)
    : QObject(parent), network_(new NetworkAccessManager(this)) {}

ConnectResponse* Client::Connect(const QString& refresh_token) {
  ConnectResponse* ret = new ConnectResponse(this);
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::LOCALHOST, this);

  if (refresh_token.isEmpty()) {
    oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, kOAuthScope);
  } else {
    oauth->RefreshAuthorisation(kOAuthTokenEndpoint, refresh_token);
  }

  NewClosure(oauth, SIGNAL(Finished()), this,
             SLOT(ConnectFinished(ConnectResponse*, OAuthenticator*)), ret,
             oauth);
  return ret;
}

void Client::ConnectFinished(ConnectResponse* response, OAuthenticator* oauth) {
  oauth->deleteLater();
  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();
  response->refresh_token_ = oauth->refresh_token();

  // Fetch user email.
  QUrl url(kGoogleOAuthUserInfoEndpoint);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchUserInfoFinished(ConnectResponse*, QNetworkReply*)),
             response, reply);
}

void Client::FetchUserInfoFinished(ConnectResponse* response,
                                   QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    qLog(Warning) << "Failed to get user info" << reply->readAll();
  } else {
    QJson::Parser parser;
    bool ok = false;
    QVariantMap result = parser.parse(reply, &ok).toMap();
    if (!ok) {
      qLog(Error) << "Failed to parse user info reply";
      return;
    }

    qLog(Debug) << result;
    response->user_email_ = result["email"].toString();
    qLog(Debug) << response->user_email_;
  }
  emit response->Finished();
  emit Authenticated();
}

void Client::AddAuthorizationHeader(QNetworkRequest* request) const {
  request->setRawHeader("Authorization",
                        QString("Bearer %1").arg(access_token_).toUtf8());
}

GetFileResponse* Client::GetFile(const QString& file_id) {
  GetFileResponse* ret = new GetFileResponse(file_id, this);

  QUrl url(QString(kGoogleDriveFile).arg(file_id));

  QNetworkRequest request = QNetworkRequest(url);
  AddAuthorizationHeader(&request);
  // Never cache these requests as we will get out of date download URLs.
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(GetFileFinished(GetFileResponse*, QNetworkReply*)), ret,
             reply);

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

ListChangesResponse* Client::ListChanges(const QString& cursor) {
  ListChangesResponse* ret = new ListChangesResponse(cursor, this);
  MakeListChangesRequest(ret);
  return ret;
}

void Client::MakeListChangesRequest(ListChangesResponse* response,
                                    const QString& page_token) {
  QUrl url(kGoogleDriveChanges);
  if (!response->cursor().isEmpty()) {
    url.addQueryItem("startChangeId", response->cursor());
  }
  if (!page_token.isEmpty()) {
    url.addQueryItem("pageToken", page_token);
  }

  qLog(Debug) << "Requesting changes at:" << response->cursor() << page_token;

  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ListChangesFinished(ListChangesResponse*, QNetworkReply*)),
             response, reply);
}

void Client::ListChangesFinished(ListChangesResponse* response,
                                 QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;
  bool ok = false;
  // TODO(John Maguire): Put this on a separate thread as the response could be large.
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Failed to fetch changes" << response->cursor();
    emit response->Finished();
    return;
  }

  if (result.contains("largestChangeId")) {
    response->next_cursor_ = result["largestChangeId"].toString();
  }

  // Emit the FilesFound signal for the files in the response.
  FileList files;
  QList<QUrl> files_deleted;
  for (const QVariant& v : result["items"].toList()) {
    QVariantMap change = v.toMap();
    if (change["deleted"].toBool() ||
        change["file"].toMap()["labels"].toMap()["trashed"].toBool()) {
      QUrl url;
      url.setScheme("googledrive");
      url.setPath(change["fileId"].toString());
      files_deleted << url;
    } else {
      files << File(change["file"].toMap());
    }
  }

  emit response->FilesFound(files);
  emit response->FilesDeleted(files_deleted);

  // Get the next page of results if there is one.
  if (result.contains("nextPageToken")) {
    MakeListChangesRequest(response, result["nextPageToken"].toString());
  } else {
    emit response->Finished();
  }
}

bool Client::is_authenticated() const {
  return !access_token_.isEmpty() &&
         QDateTime::currentDateTime().secsTo(expiry_time_) > 0;
}

void Client::ForgetCredentials() {
  access_token_ = QString();
  expiry_time_ = QDateTime();
}
