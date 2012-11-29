#include "dropboxservice.h"

#include "internet/dropboxauthenticator.h"

const char* DropboxService::kServiceName = "Dropbox";
const char* DropboxService::kSettingsGroup = "Dropbox";

namespace {

static const char* kServiceId = "dropbox";

}  // namespace

DropboxService::DropboxService(Application* app, InternetModel* parent)
    : CloudFileService(
        app, parent,
        kServiceName, kServiceId,
        QIcon(":/providers/dropbox.png"),
        SettingsDialog::Page_Dropbox) {
}

bool DropboxService::has_credentials() const {
  return !access_token_.isEmpty();
}

void DropboxService::Connect() {
  if (!has_credentials()) {
    DropboxAuthenticator* authenticator = new DropboxAuthenticator;
    authenticator->StartAuthorisation();
    NewClosure(authenticator, SIGNAL(Finished()),
               this, SLOT(AuthenticationFinished(DropboxAuthenticator*)),
               authenticator);
  } else {
    ShowSettingsDialog();
  }
}

void DropboxService::AuthenticationFinished(DropboxAuthenticator* authenticator) {
  authenticator->deleteLater();

  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  settings.setValue("access_token", authenticator->access_token());
  settings.setValue("access_token_secret", authenticator->access_token_secret());
  settings.setValue("name", authenticator->name());

  emit Connected();
}
