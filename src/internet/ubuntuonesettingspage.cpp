#include "ubuntuonesettingspage.h"

#include "ui_ubuntuonesettingspage.h"

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "internet/internetmodel.h"
#include "internet/ubuntuoneauthenticator.h"
#include "internet/ubuntuoneservice.h"

UbuntuOneSettingsPage::UbuntuOneSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::UbuntuOneSettingsPage),
      service_(dialog()->app()->internet_model()->Service<UbuntuOneService>()),
      authenticated_(false) {
  ui_->setupUi(this);
  ui_->login_state->AddCredentialGroup(ui_->login_container);
  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));

  dialog()->installEventFilter(this);
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

void UbuntuOneSettingsPage::Save() {
  QSettings s;
  s.beginGroup(UbuntuOneService::kSettingsGroup);

  if (authenticated_) {
    s.setValue("user_email", ui_->username->text());
  }
}

void UbuntuOneSettingsPage::LoginClicked() {
  ui_->login_button->setEnabled(false);
  QString username = ui_->username->text();
  QString password = ui_->password->text();

  UbuntuOneAuthenticator* authenticator = new UbuntuOneAuthenticator;
  authenticator->StartAuthorisation(username, password);
  NewClosure(authenticator, SIGNAL(Finished()),
             this, SLOT(Connected(UbuntuOneAuthenticator*)), authenticator);
  NewClosure(authenticator, SIGNAL(Finished()),
             service_, SLOT(AuthenticationFinished(UbuntuOneAuthenticator*)),
             authenticator);
}

void UbuntuOneSettingsPage::LogoutClicked() {

}

void UbuntuOneSettingsPage::Connected(UbuntuOneAuthenticator* authenticator) {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, ui_->username->text());
  authenticated_ = true;
}

bool UbuntuOneSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}
