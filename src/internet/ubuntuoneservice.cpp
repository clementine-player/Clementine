#include "ubuntuoneservice.h"

#include <QDateTime>
#include <QSettings>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "internet/internetmodel.h"
#include "internet/ubuntuoneauthenticator.h"
#include "internet/ubuntuoneurlhandler.h"

const char* UbuntuOneService::kServiceName = "Ubuntu One";
const char* UbuntuOneService::kSettingsGroup = "Ubuntu One";

namespace {
static const char* kFileStorageEndpoint =
    "https://one.ubuntu.com/api/file_storage/v1/~/Ubuntu One/";
static const char* kContentRoot = "https://files.one.ubuntu.com";
}

UbuntuOneService::UbuntuOneService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      network_(new NetworkAccessManager(this)) {
  app->player()->RegisterUrlHandler(new UbuntuOneUrlHandler(this, this));
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
    QString content_path = child["content_path"].toString();

    QUrl content_url;
    content_url.setScheme("ubuntuonefile");
    content_url.setPath(content_path);

    Song song;
    song.set_title(child["path"].toString().mid(1));
    song.set_url(content_url);

    root_->appendRow(CreateSongItem(song));
  }
}

QUrl UbuntuOneService::GetStreamingUrlFromSongId(const QString& song_id) {
  QUrl url(kContentRoot);
  url.setPath(song_id);
  url.setFragment(GenerateAuthorisationHeader());
  return url;
}
