#include "ubuntuoneservice.h"

#include <QDateTime>
#include <QSettings>
#include <QSortFilterProxyModel>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/internetmodel.h"
#include "internet/ubuntuoneauthenticator.h"
#include "internet/ubuntuoneurlhandler.h"
#include "library/librarybackend.h"
#include "playlist/playlist.h"
#include "ui/iconloader.h"

const char* UbuntuOneService::kServiceName = "Ubuntu One";
const char* UbuntuOneService::kSettingsGroup = "Ubuntu One";

namespace {
static const char* kFileStorageEndpoint =
    "https://one.ubuntu.com/api/file_storage/v1";
static const char* kVolumesEndpoint =
    "https://one.ubuntu.com/api/file_storage/v1/volumes";
static const char* kContentRoot = "https://files.one.ubuntu.com";
static const char* kServiceId = "ubuntu_one";
}

UbuntuOneService::UbuntuOneService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       QIcon(":/providers/ubuntuone.png"),
                       SettingsDialog::Page_UbuntuOne) {
  app_->player()->RegisterUrlHandler(new UbuntuOneUrlHandler(this, this));

  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.contains("consumer_key")) {
    consumer_key_ = s.value("consumer_key").toString();
    consumer_secret_ = s.value("consumer_secret").toString();
    token_ = s.value("token").toString();
    token_secret_ = s.value("token_secret").toString();
  }
}

bool UbuntuOneService::has_credentials() const {
  return !consumer_key_.isEmpty();
}

void UbuntuOneService::Connect() {
  if (has_credentials()) {
    RequestVolumeList();
  } else {
    ShowSettingsDialog();
  }
}

QByteArray UbuntuOneService::GenerateAuthorisationHeader() {
  return UbuntuOneAuthenticator::GenerateAuthorisationHeader(
      consumer_key_, consumer_secret_, token_, token_secret_);
}

void UbuntuOneService::AuthenticationFinished(
    UbuntuOneAuthenticator* authenticator) {
  authenticator->deleteLater();
  if (!authenticator->success()) {
    return;
  }

  consumer_key_ = authenticator->consumer_key();
  consumer_secret_ = authenticator->consumer_secret();
  token_ = authenticator->token();
  token_secret_ = authenticator->token_secret();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("consumer_key", consumer_key_);
  s.setValue("consumer_secret", consumer_secret_);
  s.setValue("token", token_);
  s.setValue("token_secret", token_secret_);

  RequestVolumeList();
}

QNetworkReply* UbuntuOneService::SendRequest(const QUrl& url) {
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Accept", "application/json");

  return network_->get(request);
}

void UbuntuOneService::RequestVolumeList() {
  QUrl volumes_url(kVolumesEndpoint);
  QNetworkReply* reply = SendRequest(volumes_url);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(VolumeListRequestFinished(QNetworkReply*)), reply);
}

void UbuntuOneService::VolumeListRequestFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;
  QVariantList result = parser.parse(reply).toList();
  for (const QVariant& v : result) {
    RequestFileList(v.toMap()["node_path"].toString());
  }
}

void UbuntuOneService::RequestFileList(const QString& path) {
  QUrl files_url(QString(kFileStorageEndpoint) + path);
  files_url.addQueryItem("include_children", "true");

  qLog(Debug) << "Sending files request" << files_url;
  QNetworkReply* files_reply = SendRequest(files_url);
  NewClosure(files_reply, SIGNAL(finished()), this,
             SLOT(FileListRequestFinished(QNetworkReply*)), files_reply);
}

void UbuntuOneService::FileListRequestFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap result = parser.parse(reply).toMap();

  QVariantList children = result["children"].toList();
  for (const QVariant& c : children) {
    QVariantMap child = c.toMap();
    if (child["kind"].toString() == "file") {
      QString content_path = child["content_path"].toString();
      QUrl content_url(kContentRoot);
      content_url.setPath(content_path);
      QUrl service_url;
      service_url.setScheme("ubuntuonefile");
      service_url.setPath(content_path);

      Song metadata;
      metadata.set_url(service_url);
      metadata.set_etag(child["hash"].toString());
      metadata.set_mtime(QDateTime::fromString(child["when_changed"].toString(),
                                               Qt::ISODate).toTime_t());
      metadata.set_ctime(QDateTime::fromString(child["when_created"].toString(),
                                               Qt::ISODate).toTime_t());
      metadata.set_filesize(child["size"].toInt());
      metadata.set_title(child["path"].toString().mid(1));
      MaybeAddFileToDatabase(
          metadata, GuessMimeTypeForFile(child["path"].toString().mid(1)),
          content_url, GenerateAuthorisationHeader());
    } else {
      RequestFileList(child["resource_path"].toString());
    }
  }
}

QUrl UbuntuOneService::GetStreamingUrlFromSongId(const QString& song_id) {
  QUrl url(kContentRoot);
  url.setPath(song_id);
  url.setFragment(GenerateAuthorisationHeader());
  return url;
}

void UbuntuOneService::ShowCoverManager() {
  if (!cover_manager_) {
    cover_manager_.reset(new AlbumCoverManager(app_, library_backend_));
    cover_manager_->Init();
    connect(cover_manager_.get(), SIGNAL(AddToPlaylist(QMimeData*)),
            SLOT(AddToPlaylist(QMimeData*)));
  }
  cover_manager_->show();
}

void UbuntuOneService::AddToPlaylist(QMimeData* mime) {
  playlist_manager_->current()->dropMimeData(mime, Qt::CopyAction, -1, 0,
                                             QModelIndex());
}
