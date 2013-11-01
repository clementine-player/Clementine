#include "boxservice.h"

#include <qjson/parser.h>

#include "core/application.h"
#include "core/player.h"
#include "core/waitforsignal.h"
#include "internet/boxurlhandler.h"
#include "internet/oauthenticator.h"
#include "library/librarybackend.h"

const char* BoxService::kServiceName = "Box";
const char* BoxService::kSettingsGroup = "Box";

namespace {

static const char* kClientId = "gbswb9wp7gjyldc3qrw68h2rk68jaf4h";
static const char* kClientSecret = "pZ6cUCQz5X0xaWoPVbCDg6GpmfTtz73s";

static const char* kOAuthEndpoint =
    "https://api.box.com/oauth2/authorize";
static const char* kOAuthTokenEndpoint =
    "https://api.box.com/oauth2/token";

static const char* kUserInfo =
    "https://api.box.com/2.0/users/me";
static const char* kFolderItems =
    "https://api.box.com/2.0/folders/%1/items";
static const int kRootFolderId = 0;

static const char* kFileContent =
    "https://api.box.com/2.0/files/%1/content";

static const char* kEvents =
    "https://api.box.com/2.0/events";

}

BoxService::BoxService(Application* app, InternetModel* parent)
    : CloudFileService(
        app, parent,
        kServiceName, kSettingsGroup,
        QIcon(":/providers/box.png"),
        SettingsDialog::Page_Box) {
  app->player()->RegisterUrlHandler(new BoxUrlHandler(this, this));
}

bool BoxService::has_credentials() const {
  return !refresh_token().isEmpty();
}

QString BoxService::refresh_token() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  return s.value("refresh_token").toString();
}

bool BoxService::is_authenticated() const {
  return !access_token_.isEmpty() &&
      QDateTime::currentDateTime().secsTo(expiry_time_) > 0;
}

void BoxService::EnsureConnected() {
  if (is_authenticated()) {
    return;
  }

  Connect();
  WaitForSignal(this, SIGNAL(Connected()));
}

void BoxService::Connect() {
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::REMOTE, this);
  if (!refresh_token().isEmpty()) {
    oauth->RefreshAuthorisation(
        kOAuthTokenEndpoint, refresh_token());
  } else {
    oauth->StartAuthorisation(
        kOAuthEndpoint,
        kOAuthTokenEndpoint,
        QString::null);
  }

  NewClosure(oauth, SIGNAL(Finished()),
             this, SLOT(ConnectFinished(OAuthenticator*)), oauth);
}

void BoxService::ConnectFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", oauth->refresh_token());

  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();

  if (s.value("name").toString().isEmpty()) {
    QUrl url(kUserInfo);
    QNetworkRequest request(url);
    AddAuthorizationHeader(&request);

    QNetworkReply* reply = network_->get(request);
    NewClosure(reply, SIGNAL(finished()),
               this, SLOT(FetchUserInfoFinished(QNetworkReply*)), reply);
  } else {
    emit Connected();
  }
  UpdateFiles();
}

void BoxService::AddAuthorizationHeader(QNetworkRequest* request) const {
  request->setRawHeader(
      "Authorization", QString("Bearer %1").arg(access_token_).toUtf8());
}

void BoxService::FetchUserInfoFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();

  QString name = response["name"].toString();
  if (!name.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("name", name);
  }

  emit Connected();
}

void BoxService::ForgetCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("refresh_token");
  s.remove("name");
}

void BoxService::UpdateFiles() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  if (!s.value("cursor").toString().isEmpty()) {
    // Use events API to fetch changes.
    UpdateFilesFromCursor(s.value("cursor").toString());
    return;
  }

  // First run we scan as events may not cover everything.
  FetchRecursiveFolderItems(kRootFolderId);
  InitialiseEventsCursor();
}

void BoxService::InitialiseEventsCursor() {
  QUrl url(kEvents);
  url.addQueryItem("stream_position", "now");
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(InitialiseEventsFinished(QNetworkReply*)), reply);
}

void BoxService::InitialiseEventsFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  if (response.contains("next_stream_position")) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("cursor", response["next_stream_position"]);
  }
}

void BoxService::FetchRecursiveFolderItems(const int folder_id, const int offset) {
  QUrl url(QString(kFolderItems).arg(folder_id));
  QStringList fields;
  fields << "etag"
         << "size"
         << "created_at"
         << "modified_at"
         << "name";
  QString fields_list = fields.join(",");
  url.addQueryItem("fields", fields_list);
  url.addQueryItem("limit", "1000");  // Maximum according to API docs.
  url.addQueryItem("offset", QString::number(offset));
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(FetchFolderItemsFinished(QNetworkReply*, int)),
             reply, folder_id);
}

void BoxService::FetchFolderItemsFinished(
    QNetworkReply* reply, const int folder_id) {
  reply->deleteLater();

  QByteArray data = reply->readAll();

  QJson::Parser parser;
  QVariantMap response = parser.parse(data).toMap();

  QVariantList entries = response["entries"].toList();
  const int total_entries = response["total_count"].toInt();
  const int offset = response["offset"].toInt();
  if (entries.size() + offset < total_entries) {
    // Fetch the next page if necessary.
    FetchRecursiveFolderItems(folder_id, offset + entries.size());
  }

  foreach (const QVariant& e, entries) {
    QVariantMap entry = e.toMap();
    if (entry["type"].toString() == "folder") {
      FetchRecursiveFolderItems(entry["id"].toInt());
    } else {
      MaybeAddFileEntry(entry);
    }
  }
}

void BoxService::MaybeAddFileEntry(const QVariantMap& entry) {
  QString mime_type = GuessMimeTypeForFile(entry["name"].toString());
  QUrl url;
  url.setScheme("box");
  url.setPath(entry["id"].toString());

  Song song;
  song.set_url(url);
  song.set_ctime(entry["created_at"].toDateTime().toTime_t());
  song.set_mtime(entry["modified_at"].toDateTime().toTime_t());
  song.set_filesize(entry["size"].toInt());
  song.set_title(entry["name"].toString());

  // This is actually a redirect. Follow it now.
  QNetworkReply* reply = FetchContentUrlForFile(entry["id"].toString());
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RedirectFollowed(QNetworkReply*, Song, QString)),
             reply, song, mime_type);
}

QNetworkReply* BoxService::FetchContentUrlForFile(const QString& file_id) {
  QUrl content_url(QString(kFileContent).arg(file_id));
  QNetworkRequest request(content_url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  return reply;
}

void BoxService::RedirectFollowed(
    QNetworkReply* reply, const Song& song, const QString& mime_type) {
  reply->deleteLater();
  QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (!redirect.isValid()) {
    return;
  }

  QUrl real_url = redirect.toUrl();
  MaybeAddFileToDatabase(
      song,
      mime_type,
      real_url,
      QString("Bearer %1").arg(access_token_));
}

void BoxService::UpdateFilesFromCursor(const QString& cursor) {
  QUrl url(kEvents);
  url.addQueryItem("stream_position", cursor);
  url.addQueryItem("limit", "5000");
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(FetchEventsFinished(QNetworkReply*)), reply);
}

void BoxService::FetchEventsFinished(QNetworkReply* reply) {
  // TODO: Page through events.
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("cursor", response["next_stream_position"]);

  QVariantList entries = response["entries"].toList();
  foreach (const QVariant& e, entries) {
    QVariantMap event = e.toMap();
    QString type = event["event_type"].toString();
    QVariantMap source = event["source"].toMap();
    if (source["type"] == "file") {
      if (type == "ITEM_UPLOAD") {
        // Add file.
        MaybeAddFileEntry(source);
      } else if (type == "ITEM_TRASH") {
        // Delete file.
        QUrl url;
        url.setScheme("box");
        url.setPath(source["id"].toString());
        Song song = library_backend_->GetSongByUrl(url);
        if (song.is_valid()) {
          library_backend_->DeleteSongs(SongList() << song);
        }
      }
    }
  }
}

QUrl BoxService::GetStreamingUrlFromSongId(const QString& id) {
  EnsureConnected();
  QNetworkReply* reply = FetchContentUrlForFile(id);
  WaitForSignal(reply, SIGNAL(finished()));
  reply->deleteLater();
  QUrl real_url =
      reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  return real_url;
}
