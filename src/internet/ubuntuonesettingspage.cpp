#include "ubuntuonesettingspage.h"

#include "ui_ubuntuonesettingspage.h"

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "internet/internetmodel.h"
#include "internet/ubuntuoneauthenticator.h"
#include "internet/ubuntuoneservice.h"

#include <QMessageBox>

UbuntuOneSettingsPage::UbuntuOneSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::UbuntuOneSettingsPage),
      service_(dialog()->app()->internet_model()->Service<UbuntuOneService>()),
      authenticated_(false) {
  ui_->setupUi(this);

  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(LoginClicked()));
  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
}

void UbuntuOneSettingsPage::Load() {
  QSettings s;
  s.beginGroup(UbuntuOneService::kSettingsGroup);

  const QString user_email = s.value("user_email").toString();
  if (!user_email.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, user_email);
    ui_->username->setText(user_email);
  }
}

void UbuntuOneSettingsPage::Save() {}

void UbuntuOneSettingsPage::LoginClicked() {
  const QString username = ui_->username->text();
  const QString password = ui_->password->text();
  ui_->password->clear();

  UbuntuOneAuthenticator* authenticator = new UbuntuOneAuthenticator;
  authenticator->StartAuthorisation(username, password);
  NewClosure(authenticator, SIGNAL(Finished()), this,
             SLOT(Connected(UbuntuOneAuthenticator*)), authenticator);
  NewClosure(authenticator, SIGNAL(Finished()), service_,
             SLOT(AuthenticationFinished(UbuntuOneAuthenticator*)),
             authenticator);

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
}

void UbuntuOneSettingsPage::LogoutClicked() {
  QSettings s;
  s.beginGroup(UbuntuOneService::kSettingsGroup);
  s.remove("user_email");
  s.remove("consumer_key");
  s.remove("consumer_secret");
  s.remove("token");
  s.remove("token_secret");

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void UbuntuOneSettingsPage::Connected(UbuntuOneAuthenticator* authenticator) {
  if (!authenticator->success()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
    QMessageBox::warning(this, tr("Authentication failed"),
                         tr("Your username or password was incorrect."));
    return;
  }

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn,
                                ui_->username->text());
  authenticated_ = true;

  QSettings s;
  s.beginGroup(UbuntuOneService::kSettingsGroup);
  s.setValue("user_email", ui_->username->text());
}
