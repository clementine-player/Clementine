#include "dropboxservice.h"

#include "core/logging.h"
#include "core/network.h"
#include "internet/dropboxauthenticator.h"

const char* DropboxService::kServiceName = "Dropbox";
const char* DropboxService::kSettingsGroup = "Dropbox";

namespace {

static const char* kServiceId = "dropbox";

static const char* kMetadataEndpoint =
    "https://api.dropbox.com/1/metadata/dropbox/";
//static const char* kMediaEndpoint =
//    "https://api.dropbox.com/1/media/dropbox/";

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
}

bool DropboxService::has_credentials() const {
  return !access_token_.isEmpty();
}

void DropboxService::Connect() {
  if (has_credentials()) {
    RequestFileList("");
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

  RequestFileList("");
}

QByteArray DropboxService::GenerateAuthorisationHeader() {
  return DropboxAuthenticator::GenerateAuthorisationHeader(
      access_token_,
      access_token_secret_);
}

void DropboxService::RequestFileList(const QString& path) {
  QUrl url(QString(kMetadataEndpoint) + path);
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RequestFileListFinished(QNetworkReply*)), reply);
}

void DropboxService::RequestFileListFinished(QNetworkReply* reply) {
  reply->deleteLater();
  qLog(Debug) << reply->readAll();
}
