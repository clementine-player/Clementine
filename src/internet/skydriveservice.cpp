#include "skydriveservice.h"

#include "oauthenticator.h"

namespace {

static const char* kServiceName = "Skydrive";
static const char* kServiceId = "skydrive";
static const char* kSettingsGroup = "Skydrive";

static const char* kClientId = "00000000400E7C78";
static const char* kClientSecret = "B0KLZjEgC5SpW0KknrsBFwlaKmGThaAk";

static const char* kOAuthEndpoint =
    "https://login.live.com/oauth20_authorize.srf";
static const char* kOAuthTokenEndpoint =
    "https://login.live.com/oauth20_token.srf";
static const char* kOAuthScope = "wl.basic wl.skydrive wl.offline_access";

}  // namespace

SkydriveService::SkydriveService(
    Application* app,
    InternetModel* parent)
  : CloudFileService(
      app, parent, kServiceName, kServiceId,
      QIcon(":providers/skydrive.png"), SettingsDialog::Page_Skydrive) {
}

bool SkydriveService::has_credentials() const {
  return true;
}

void SkydriveService::Connect() {
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::REMOTE, this);
  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.contains("refresh_token")) {
    oauth->RefreshAuthorisation(
        kOAuthTokenEndpoint, s.value("refresh_token").toString());
  } else {
    oauth->StartAuthorisation(
        kOAuthEndpoint,
        kOAuthTokenEndpoint,
        kOAuthScope);
  }

  NewClosure(oauth, SIGNAL(Finished()),
             this, SLOT(ConnectFinished(OAuthenticator*)), oauth);
}

void SkydriveService::ConnectFinished(OAuthenticator* oauth) {
  qLog(Debug) << oauth->access_token()
              << oauth->refresh_token();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("refresh_token", oauth->refresh_token());
}
