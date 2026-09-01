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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QUrlQuery>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "internet/core/oauthenticator.h"

using namespace google_drive;

namespace {
static const char* kGoogleDriveFile =
    "https://www.googleapis.com/drive/v3/files/%1";
static const char* kGoogleDriveChanges =
    "https://www.googleapis.com/drive/v3/changes";
static const char* kGoogleDriveStartPageToken =
    "https://www.googleapis.com/drive/v3/changes/startPageToken";
// Fields requested on every file resource we read - see File's accessors.
static const char* kFileFields =
    "id,name,mimeType,description,size,modifiedTime,createdTime,version";
// Introspects a token to find out what it's for - works for any valid
// token regardless of scope, unlike the old oauth2/v1/userinfo endpoint,
// which needed the userinfo.email scope we can no longer request (see
// kOAuthScope below).
static const char* kGoogleTokenInfoEndpoint =
    "https://www.googleapis.com/oauth2/v1/tokeninfo";

static const char* kOAuthEndpoint = "https://accounts.google.com/o/oauth2/auth";
static const char* kOAuthTokenEndpoint =
    "https://accounts.google.com/o/oauth2/token";
// drive.file only grants access to files the user explicitly picks
// (see Client::AuthorizeAndPick) - unlike drive.readonly it is not a
// sensitive scope, so it doesn't require Google app verification. Google's
// desktop-app Picker flow requires this to be the only scope requested:
// https://developers.google.com/workspace/drive/picker/guides/desktop-mobile-picker
static const char* kOAuthScope = "https://www.googleapis.com/auth/drive.file";
static const char* kClientId = "679260893280.apps.googleusercontent.com";
static const char* kClientSecret = "l3cWb8efUZsrBI4wmY3uKl6i";
}  // namespace

QUrl File::download_url() const {
  QUrl url(QString(kGoogleDriveFile).arg(id()));
  QUrlQuery url_query;
  url_query.addQueryItem("alt", "media");
  // Same as GetFile()'s metadata request: content requests for a file
  // inside a Shared Drive 404 without this even with a valid grant.
  url_query.addQueryItem("supportsAllDrives", "true");
  url.setQuery(url_query);
  return url;
}

ConnectResponse::ConnectResponse(QObject* parent) : QObject(parent) {}

GetFileResponse::GetFileResponse(const QString& file_id, QObject* parent)
    : QObject(parent), file_id_(file_id), had_error_(false) {}

ListChangesResponse::ListChangesResponse(const QString& cursor, QObject* parent)
    : QObject(parent), cursor_(cursor) {}

Client::Client(QObject* parent, QNetworkAccessManager* network)
    : QObject(parent),
      network_(network ? network : new NetworkAccessManager(this)) {}

ConnectResponse* Client::Connect(const QString& refresh_token) {
  ConnectResponse* ret = new ConnectResponse(this);
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::LOCALHOST, this);

  oauth->RefreshAuthorisation(kOAuthTokenEndpoint, refresh_token);

  NewClosure(
      oauth, SIGNAL(Finished()), this,
      SLOT(ConnectFinished(ConnectResponse*, OAuthenticator*)),
      ret, oauth);
  return ret;
}

ConnectResponse* Client::AuthorizeAndPick(const QStringList& mime_types) {
  ConnectResponse* ret = new ConnectResponse(this);
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::LOCALHOST, this);

  QUrlQuery extra_params;
  extra_params.addQueryItem("prompt", "consent");
  extra_params.addQueryItem("trigger_onepick", "true");
  extra_params.addQueryItem("allow_multiple", "true");
  if (!mime_types.isEmpty()) {
    extra_params.addQueryItem("mimetypes", mime_types.join(","));
  }

  oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, kOAuthScope,
                            extra_params);

  NewClosure(
      oauth, SIGNAL(Finished()), this,
      SLOT(ConnectFinished(ConnectResponse*, OAuthenticator*)),
      ret, oauth);
  return ret;
}

void Client::ConnectFinished(ConnectResponse* response, OAuthenticator* oauth) {
  oauth->deleteLater();
  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();
  response->refresh_token_ = oauth->refresh_token();
  response->picked_file_ids_ =
      oauth->picked_file_ids().split(',', Qt::SkipEmptyParts);

  // Introspect the token to get the user's email.
  QUrl url(kGoogleTokenInfoEndpoint);
  QUrlQuery url_query;
  url_query.addQueryItem("access_token", access_token_);
  url.setQuery(url_query);
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
            SLOT(FetchUserInfoFinished(ConnectResponse*, QNetworkReply*)),
            response, reply);
}

void Client::FetchUserInfoFinished(ConnectResponse* response,
                                   QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    qLog(Warning) << "Failed to get token info" << reply->readAll();
  } else {
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
      qLog(Error) << "Failed to parse token info reply";
      return;
    }

    response->user_email_ = document.object()["email"].toString();
  }
  emit response->Finished();
  emit Authenticated();
}

QByteArray Client::GetAuthHeader() const {
  return QString("Bearer %1").arg(access_token_).toUtf8();
}

void Client::AddAuthorizationHeader(QNetworkRequest* request) const {
  request->setRawHeader("Authorization", GetAuthHeader());
}

void Client::AddResourceKeyHeader(QNetworkRequest* request,
                                  const QString& file_id,
                                  const QString& resource_key) const {
  if (resource_key.isEmpty()) {
    return;
  }
  request->setRawHeader(
      "X-Goog-Drive-Resource-Keys",
      QString("%1/%2").arg(file_id, resource_key).toUtf8());
}

GetFileResponse* Client::GetFile(const QString& file_id,
                                 const QString& resource_key) {
  GetFileResponse* ret = new GetFileResponse(file_id, this);

  QUrl url(QString(kGoogleDriveFile).arg(file_id));
  QUrlQuery url_query(url);
  // Requests for files inside a Shared Drive 404 without this even with a
  // valid grant.
  url_query.addQueryItem("supportsAllDrives", "true");
  url_query.addQueryItem("fields", kFileFields);
  url.setQuery(url_query);

  qLog(Debug) << "GetFile" << file_id << "url =" << url
             << ", resource key =" << resource_key;

  QNetworkRequest request = QNetworkRequest(url);
  AddAuthorizationHeader(&request);
  AddResourceKeyHeader(&request, file_id, resource_key);
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

  const QByteArray data = reply->readAll();
  const QVariant status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  qLog(Debug) << "GetFile" << response->file_id_ << "-> HTTP status" << status
             << ", url =" << reply->url() << ", body =" << data;

  if (status != 200) {
    // A Drive API error is still valid JSON (eg. {"error": {...}}), so this
    // has to be checked before treating a parseable body as success -
    // otherwise callers see an empty-but-"successful" File.
    qLog(Error) << "Failed to fetch file with ID" << response->file_id_ << data;
    qLog(Debug) << "GetFile" << response->file_id_
               << "response headers:" << reply->rawHeaderPairs()
               << ", request had Authorization header:"
               << reply->request().hasRawHeader("Authorization")
               << ", request had resource key header:"
               << reply->request().hasRawHeader("X-Goog-Drive-Resource-Keys");
    response->had_error_ = true;
    emit response->Finished();
    return;
  }

  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Failed to fetch file with ID" << response->file_id_;
    response->had_error_ = true;
    emit response->Finished();
    return;
  }

  response->file_ = File(document.object().toVariantMap());
  emit response->Finished();
}

ListChangesResponse* Client::ListChanges(const QString& cursor) {
  ListChangesResponse* ret = new ListChangesResponse(cursor, this);
  if (cursor.isEmpty()) {
    RequestStartPageToken(ret);
  } else {
    MakeListChangesRequest(ret, cursor);
  }
  return ret;
}

void Client::RequestStartPageToken(ListChangesResponse* response) {
  QUrl url(kGoogleDriveStartPageToken);
  QUrlQuery url_query;
  url_query.addQueryItem("supportsAllDrives", "true");
  url.setQuery(url_query);

  qLog(Debug) << "Requesting a Google Drive change cursor:" << url;

  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
            SLOT(StartPageTokenFinished(ListChangesResponse*, QNetworkReply*)),
            response, reply);
}

void Client::StartPageTokenFinished(ListChangesResponse* response,
                                    QNetworkReply* reply) {
  reply->deleteLater();

  const QByteArray data = reply->readAll();
  const QVariant status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  qLog(Debug) << "startPageToken -> HTTP status" << status
             << ", body =" << data;

  if (status != 200) {
    qLog(Error) << "Failed to get a Google Drive change cursor" << data;
    emit response->Finished();
    return;
  }

  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Failed to parse startPageToken reply";
    emit response->Finished();
    return;
  }

  response->next_cursor_ = document.object().value("startPageToken").toString();
  qLog(Debug) << "Established Google Drive change cursor:"
             << response->next_cursor_;
  emit response->Finished();
}

void Client::MakeListChangesRequest(ListChangesResponse* response,
                                    const QString& page_token) {
  QUrl url(kGoogleDriveChanges);
  QUrlQuery url_query;
  url_query.addQueryItem("pageToken", page_token);
  // Changes to files inside a Shared Drive aren't reported without these.
  url_query.addQueryItem("supportsAllDrives", "true");
  url_query.addQueryItem("includeItemsFromAllDrives", "true");
  url_query.addQueryItem(
      "fields",
      QString("nextPageToken,newStartPageToken,changes(fileId,removed,"
             "file(%1,trashed))")
          .arg(kFileFields));

  url.setQuery(url_query);

  qLog(Debug) << "Requesting changes at page token:" << page_token
             << ", url =" << url;

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

  const QByteArray data = reply->readAll();
  const QVariant status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  qLog(Debug) << "ListChanges cursor =" << response->cursor()
             << "-> HTTP status" << status << ", url =" << reply->url();

  if (status != 200) {
    // A Drive API error is still valid JSON, so this has to be checked
    // before parsing - otherwise an error response is silently treated as
    // "zero changes" instead of being reported.
    qLog(Error) << "Failed to fetch changes" << response->cursor() << data;
    emit response->Finished();
    return;
  }

  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(data, &error);
  // TODO(John Maguire): Put this on a separate thread as the response could be
  // large.
  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Failed to fetch changes" << response->cursor();
    emit response->Finished();
    return;
  }

  QJsonObject json_result = document.object();
  // Only present on the last page.
  if (json_result.contains("newStartPageToken")) {
    response->next_cursor_ = json_result.value("newStartPageToken").toString();
  }

  // Emit the FilesFound signal for the files in the response.
  FileList files;
  QList<QUrl> files_deleted;
  for (const QJsonValue& v : json_result.value("changes").toArray()) {
    QJsonObject change = v.toObject();
    QJsonObject file = change.value("file").toObject();
    if (change.value("removed").toBool() || file.value("trashed").toBool()) {
      QUrl url;
      url.setScheme("googledrive");
      url.setPath("/" + change.value("fileId").toString());
      files_deleted << url;
    } else {
      files << File(file.toVariantMap());
    }
  }

  qLog(Debug) << "ListChanges parsed"
             << json_result.value("changes").toArray().size()
             << "item(s):" << files.size() << "found," << files_deleted.size()
             << "deleted; newStartPageToken ="
             << json_result.value("newStartPageToken").toString()
             << ", nextPageToken ="
             << json_result.value("nextPageToken").toString();

  emit response->FilesFound(files);
  emit response->FilesDeleted(files_deleted);

  // Get the next page of results if there is one.
  if (json_result.contains("nextPageToken")) {
    MakeListChangesRequest(response, json_result["nextPageToken"].toString());
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
