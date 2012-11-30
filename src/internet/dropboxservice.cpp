#include "dropboxservice.h"

#include <QFileInfo>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/utilities.h"
#include "core/waitforsignal.h"
#include "internet/dropboxauthenticator.h"
#include "internet/dropboxurlhandler.h"
#include "library/librarybackend.h"

using Utilities::ParseRFC822DateTime;

const char* DropboxService::kServiceName = "Dropbox";
const char* DropboxService::kSettingsGroup = "Dropbox";

namespace {

static const char* kServiceId = "dropbox";

static const char* kMediaEndpoint =
    "https://api.dropbox.com/1/media/dropbox/";
static const char* kDeltaEndpoint =
    "https://api.dropbox.com/1/delta";

}  // namespace

DropboxService::DropboxService(Application* app, InternetModel* parent)
    : CloudFileService(
        app, parent,
        kServiceName, kServiceId,
        QIcon(":/providers/dropbox.png"),
        SettingsDialog::Page_Dropbox),
      network_(new NetworkAccessManager(this)) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  access_token_ = settings.value("access_token").toString();
  access_token_secret_ = settings.value("access_token_secret").toString();
  app->player()->RegisterUrlHandler(new DropboxUrlHandler(this, this));
}

bool DropboxService::has_credentials() const {
  return !access_token_.isEmpty();
}

void DropboxService::Connect() {
  if (has_credentials()) {
    RequestFileList();
  } else {
    ShowSettingsDialog();
  }
}

void DropboxService::AuthenticationFinished(DropboxAuthenticator* authenticator) {
  authenticator->deleteLater();

  access_token_ = authenticator->access_token();
  access_token_secret_ = authenticator->access_token_secret();

  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  settings.setValue("access_token", access_token_);
  settings.setValue("access_token_secret", access_token_secret_);
  settings.setValue("name", authenticator->name());

  emit Connected();

  RequestFileList();
}

QByteArray DropboxService::GenerateAuthorisationHeader() {
  return DropboxAuthenticator::GenerateAuthorisationHeader(
      access_token_,
      access_token_secret_);
}

void DropboxService::RequestFileList() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QUrl url = QUrl(QString(kDeltaEndpoint));
  if (s.contains("cursor")) {
    url.addQueryItem("cursor", s.value("cursor").toString());
  }
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RequestFileListFinished(QNetworkReply*)), reply);
}

namespace {

bool IsSupportedMimeType(const QString& mime_type) {
  return mime_type == "audio/ogg" ||
         mime_type == "audio/mpeg";
}

}  // namespace

void DropboxService::RequestFileListFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  if (response.contains("reset") &&
      response["reset"].toBool()) {
    qLog(Debug) << "Resetting Dropbox DB";
    library_backend_->DeleteAll();
  }

  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("cursor", response["cursor"].toString());

  QVariantList contents = response["entries"].toList();
  qLog(Debug) << "Delta found:" << contents.size();
  foreach (const QVariant& c, contents) {
    QVariantList item = c.toList();
    QString path = item[0].toString();

    QUrl url;
    url.setScheme("dropbox");
    url.setPath(path);

    if (item[1].isNull()) {
      // Null metadata indicates path deleted.
      qLog(Debug) << "Deleting:" << url;
      Song song = library_backend_->GetSongByUrl(url);
      if (song.is_valid()) {
        library_backend_->DeleteSongs(SongList() << song);
      }
      continue;
    }

    QVariantMap metadata = item[1].toMap();
    if (metadata["is_dir"].toBool()) {
      continue;
    }
    MaybeAddFileToDatabase(url, metadata);
  }

  if (response.contains("has_more") && response["has_more"].toBool()) {
    RequestFileList();
  }
}

void DropboxService::MaybeAddFileToDatabase(
    const QUrl& url, const QVariantMap& file) {
  if (!IsSupportedMimeType(file["mime_type"].toString())) {
    return;
  }
  Song song = library_backend_->GetSongByUrl(url);
  if (song.is_valid()) {
    return;
  }

  QNetworkReply* reply = FetchContentUrl(url);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(FetchContentUrlFinished(QNetworkReply*, QVariantMap)),
             reply, file);
}

QNetworkReply* DropboxService::FetchContentUrl(const QUrl& url) {
  QUrl request_url(QString(kMediaEndpoint) + url.path());
  QNetworkRequest request(request_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  return network_->post(request, QByteArray());
}

void DropboxService::FetchContentUrlFinished(
    QNetworkReply* reply, const QVariantMap& data) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  QFileInfo info(data["path"].toString());
  TagReaderClient::ReplyType* tag_reply = app_->tag_reader_client()->ReadCloudFile(
      response["url"].toUrl(),
      info.fileName(),
      data["bytes"].toInt(),
      data["mime_type"].toString(),
      QString::null);
  NewClosure(tag_reply, SIGNAL(Finished(bool)),
      this, SLOT(ReadTagsFinished(TagReaderClient::ReplyType*,QVariantMap)),
      tag_reply, data);
}

void DropboxService::ReadTagsFinished(
    TagReaderClient::ReplyType* reply, const QVariantMap& file) {
  qLog(Debug) << reply->message().DebugString().c_str();

  const auto& message = reply->message().read_cloud_file_response();
  if (!message.has_metadata() ||
      !message.metadata().filesize()) {
    qLog(Debug) << "Failed to tag:" << file["path"].toString();
    return;
  }

  Song song;
  song.InitFromProtobuf(message.metadata());
  song.set_directory_id(0);
  song.set_etag(file["rev"].toString());
  song.set_mtime(ParseRFC822DateTime(file["modified"].toString()).toTime_t());
  QUrl url;
  url.setScheme("dropbox");
  url.setPath(file["path"].toString());
  song.set_url(url);
  if (song.title().isEmpty()) {
    QFileInfo info(file["path"].toString());
    song.set_title(info.fileName());
  }

  qLog(Debug) << "Adding song to db:" << song.title();
  library_backend_->AddOrUpdateSongs(SongList() << song);
}

QUrl DropboxService::GetStreamingUrlFromSongId(const QUrl& url) {
  QNetworkReply* reply = FetchContentUrl(url);
  WaitForSignal(reply, SIGNAL(finished()));

  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  return response["url"].toUrl();
}
