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

const char* UbuntuOneService::kServiceName = "Ubuntu One";
const char* UbuntuOneService::kSettingsGroup = "Ubuntu One";

namespace {
static const char* kFileStorageEndpoint =
    "https://one.ubuntu.com/api/file_storage/v1/~/Ubuntu One/";
static const char* kContentRoot = "https://files.one.ubuntu.com";
static const char* kSongsTable = "ubuntu_one_songs";
static const char* kFtsTable = "ubuntu_one_songs_fts";
}

UbuntuOneService::UbuntuOneService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      network_(new NetworkAccessManager(this)),
      library_sort_model_(new QSortFilterProxyModel(this)) {
  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(
      app->database(), kSongsTable, QString::null, QString::null, kFtsTable);
  library_model_ = new LibraryModel(library_backend_, app_, this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  app->player()->RegisterUrlHandler(new UbuntuOneUrlHandler(this, this));
  app->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_,
      kServiceName,
      "ubuntu_one",
      QIcon(":/providers/ubuntuone.png"),
      true, app_, this));
}

QStandardItem* UbuntuOneService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(), "Ubuntu One");
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void UbuntuOneService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      Connect();
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void UbuntuOneService::Connect() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.contains("consumer_key")) {
    consumer_key_ = s.value("consumer_key").toString();
    consumer_secret_ = s.value("consumer_secret").toString();
    token_ = s.value("token").toString();
    token_secret_ = s.value("token_secret").toString();

    RequestFileList();
    return;
  }

  UbuntuOneAuthenticator* authenticator = new UbuntuOneAuthenticator;
  authenticator->StartAuthorisation(
      "Username",
      "Password");
  NewClosure(authenticator, SIGNAL(Finished()),
             this, SLOT(AuthenticationFinished(UbuntuOneAuthenticator*)),
             authenticator);
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

  RequestFileList();
}

void UbuntuOneService::RequestFileList() {
  QUrl files_url(kFileStorageEndpoint);
  files_url.addQueryItem("include_children", "true");
  QNetworkRequest request(files_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Accept", "application/json");

  qLog(Debug) << "Sending files request";
  QNetworkReply* files_reply = network_->get(request);
  NewClosure(files_reply, SIGNAL(finished()),
             this, SLOT(FileListRequestFinished(QNetworkReply*)), files_reply);
}

void UbuntuOneService::FileListRequestFinished(QNetworkReply* reply) {
  QByteArray data = reply->readAll();
  qLog(Debug) << reply->url();
  qLog(Debug) << data;
  qLog(Debug) << reply->rawHeaderList();
  qLog(Debug) << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  qLog(Debug) << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);

  QJson::Parser parser;
  QVariantMap result = parser.parse(data).toMap();

  QVariantList children = result["children"].toList();
  for (const QVariant& c : children) {
    QVariantMap child = c.toMap();
    MaybeAddFileToDatabase(child);
  }
}

void UbuntuOneService::MaybeAddFileToDatabase(const QVariantMap& file) {
  QString content_path = file["content_path"].toString();

  QUrl service_url;
  service_url.setScheme("ubuntuonefile");
  service_url.setPath(content_path);
  Song song = library_backend_->GetSongByUrl(service_url);
  if (song.is_valid()) {
    return;
  }

  QUrl content_url(kContentRoot);
  content_url.setPath(content_path);

  TagReaderClient::ReplyType* reply = app_->tag_reader_client()->ReadGoogleDrive(
      content_url,
      file["path"].toString().mid(1),
      file["size"].toInt(),
      "audio/mpeg",
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
  song.InitFromProtobuf(reply->message().read_google_drive_response().metadata());
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
