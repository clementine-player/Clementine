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
static const char* kContentRoot = "https://files.one.ubuntu.com";
static const char* kServiceId = "ubuntu_one";
}

UbuntuOneService::UbuntuOneService(Application* app, InternetModel* parent)
    : CloudFileService(
        app, parent,
        kServiceName, kServiceId,
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
    RequestFileList("/~/Ubuntu One");
  } else {
    ShowSettingsDialog();
  }
}

QByteArray UbuntuOneService::GenerateAuthorisationHeader() {
  return UbuntuOneAuthenticator::GenerateAuthorisationHeader(
      consumer_key_,
      consumer_secret_,
      token_,
      token_secret_);
}

void UbuntuOneService::AuthenticationFinished(
    UbuntuOneAuthenticator* authenticator) {
  authenticator->deleteLater();

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

  RequestFileList("/~/Ubuntu One");
}

void UbuntuOneService::RequestFileList(const QString& path) {
  QUrl files_url(QString(kFileStorageEndpoint) + path);
  files_url.addQueryItem("include_children", "true");
  QNetworkRequest request(files_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Accept", "application/json");

  qLog(Debug) << "Sending files request" << files_url;
  QNetworkReply* files_reply = network_->get(request);
  NewClosure(files_reply, SIGNAL(finished()),
             this, SLOT(FileListRequestFinished(QNetworkReply*)), files_reply);
}

void UbuntuOneService::FileListRequestFinished(QNetworkReply* reply) {
  QByteArray data = reply->readAll();

  QJson::Parser parser;
  QVariantMap result = parser.parse(data).toMap();

  QVariantList children = result["children"].toList();
  foreach (const QVariant& c, children) {
    QVariantMap child = c.toMap();
    if (child["kind"].toString() == "file") {
      MaybeAddFileToDatabase(child);
    } else {
      RequestFileList(child["resource_path"].toString());
    }
  }
}

namespace {

QString GuessMimeTypeForFile(const QString& filename) {
  if (filename.endsWith(".mp3")) {
    return "audio/mpeg";
  } else if (filename.endsWith(".m4a")) {
    return "audio/mpeg";
  } else if (filename.endsWith(".ogg")) {
    return "application/ogg";
  } else if (filename.endsWith(".flac")) {
    return "application/x-flac";
  }
  return QString::null;
}

}  // namespace

void UbuntuOneService::MaybeAddFileToDatabase(const QVariantMap& file) {
  const QString content_path = file["content_path"].toString();
  const QString filename = file["path"].toString().mid(1);
  const QString mime_type = GuessMimeTypeForFile(filename);
  if (mime_type.isNull()) {
    // Unknown file type.
    // Potentially, we could do a HEAD request to see what Ubuntu One thinks
    // the Content-Type is, probably not worth it though.
    return;
  }

  QUrl service_url;
  service_url.setScheme("ubuntuonefile");
  service_url.setPath(content_path);
  Song song = library_backend_->GetSongByUrl(service_url);
  if (song.is_valid()) {
    return;
  }

  QUrl content_url(kContentRoot);
  content_url.setPath(content_path);

  TagReaderClient::ReplyType* reply = app_->tag_reader_client()->ReadCloudFile(
      content_url,
      filename,
      file["size"].toInt(),
      mime_type,
      GenerateAuthorisationHeader());
  NewClosure(
      reply, SIGNAL(Finished(bool)),
      this, SLOT(ReadTagsFinished(TagReaderClient::ReplyType*,QVariantMap, QUrl)),
      reply, file, service_url);
}

void UbuntuOneService::ReadTagsFinished(
    TagReaderClient::ReplyType* reply, const QVariantMap& file, const QUrl& url) {
  qLog(Debug) << reply->message().DebugString().c_str();
  Song song;
  song.InitFromProtobuf(reply->message().read_cloud_file_response().metadata());
  song.set_directory_id(0);
  song.set_etag(file["hash"].toString());
  song.set_mtime(
      QDateTime::fromString(file["when_changed"].toString(), Qt::ISODate).toTime_t());
  song.set_ctime(
      QDateTime::fromString(file["when_created"].toString(), Qt::ISODate).toTime_t());

  song.set_url(url);

  if (song.title().isEmpty()) {
    song.set_title(file["path"].toString().mid(1));
  }

  qLog(Debug) << "Adding song to db:" << song.title();
  library_backend_->AddOrUpdateSongs(SongList() << song);
}

QUrl UbuntuOneService::GetStreamingUrlFromSongId(const QString& song_id) {
  QUrl url(kContentRoot);
  url.setPath(song_id);
  url.setFragment(GenerateAuthorisationHeader());
  return url;
}

void UbuntuOneService::ShowSettingsDialog() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_UbuntuOne);
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
  playlist_manager_->current()->dropMimeData(
      mime, Qt::CopyAction, -1, 0, QModelIndex());
}
