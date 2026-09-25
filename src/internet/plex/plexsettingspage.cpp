/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plexsettingspage.h"

#include <QSettings>

#include "internet/core/internetmodel.h"
#include "internet/plex/plexauthenticator.h"
#include "ui/iconloader.h"
#include "ui_plexsettingspage.h"

PlexSettingsPage::PlexSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_PlexSettingsPage),
      service_(InternetModel::Service<PlexService>()),
      authenticator_(new PlexAuthenticator(this)) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("plex", IconLoader::Provider));

  connect(ui_->sign_in, &QPushButton::clicked, this, &PlexSettingsPage::SignIn);
  connect(ui_->check_pin, &QPushButton::clicked, this,
          &PlexSettingsPage::CheckPin);
  connect(ui_->login_state, &LoginStateWidget::LogoutClicked, this,
          &PlexSettingsPage::SignOut);

  connect(authenticator_, &PlexAuthenticator::AuthCodeReady, this,
          &PlexSettingsPage::AuthCodeReady);
  connect(authenticator_, &PlexAuthenticator::Authenticated, this,
          &PlexSettingsPage::Authenticated);
  connect(authenticator_, &PlexAuthenticator::ServersFound, this,
          &PlexSettingsPage::ServersFound);
  connect(authenticator_, &PlexAuthenticator::Failed, this,
          &PlexSettingsPage::AuthFailed);
  connect(service_, &PlexService::LoginStateChanged, this,
          &PlexSettingsPage::LoginStateChanged);

  ui_->login_state->AddCredentialGroup(ui_->sign_in_group);
  ui_->check_pin->hide();
}

PlexSettingsPage::~PlexSettingsPage() { delete ui_; }

void PlexSettingsPage::Load() {
  QSettings s;
  s.beginGroup(PlexService::kSettingsGroup);

  user_token_ = s.value("user_token").toString();
  ui_->verify_cert->setChecked(s.value("verify_cert", true).toBool());

  // Show the saved server straight away; the full list is refreshed below.
  servers_.clear();
  ui_->server->clear();
  PlexServer saved;
  saved.machine_id = s.value("server_id").toString();
  saved.name = s.value("server_name").toString();
  saved.access_token = s.value("server_token").toString();
  saved.connections =
      PlexParser::ParseConnections(s.value("connections").toByteArray());
  if (!saved.machine_id.isEmpty()) {
    servers_ << saved;
    ui_->server->addItem(saved.name);
  }

  UpdateSignedInState();
  LoginStateChanged(service_->login_state());

  if (!user_token_.isEmpty()) authenticator_->FetchServers(user_token_);
}

void PlexSettingsPage::Save() {
  QSettings s;
  s.beginGroup(PlexService::kSettingsGroup);

  s.setValue("verify_cert", ui_->verify_cert->isChecked());

  if (user_token_.isEmpty()) {
    s.remove("user_token");
  } else {
    s.setValue("user_token", user_token_);
  }

  const int index = ui_->server->currentIndex();
  if (user_token_.isEmpty() || index < 0 || index >= servers_.size()) {
    s.remove("server_id");
    s.remove("server_name");
    s.remove("server_token");
    s.remove("connections");
    return;
  }

  const PlexServer& server = servers_[index];
  s.setValue("server_id", server.machine_id);
  s.setValue("server_name", server.name);
  s.setValue("server_token", server.access_token);
  s.setValue("connections",
             PlexParser::SerializeConnections(server.connections));
}

void PlexSettingsPage::SignIn() {
  ui_->sign_in_status->setText(tr("Contacting plex.tv..."));
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
  authenticator_->StartSignIn();
}

void PlexSettingsPage::CheckPin() { authenticator_->CheckPin(); }

void PlexSettingsPage::SignOut() {
  user_token_.clear();
  servers_.clear();
  ui_->server->clear();
  UpdateSignedInState();
}

void PlexSettingsPage::AuthCodeReady(const QString& code) {
  ui_->sign_in_status->setText(
      tr("Your browser should now show plex.tv. If it asks for a code, enter "
         "<b>%1</b>.")
          .arg(code));
  ui_->check_pin->show();
}

void PlexSettingsPage::Authenticated(const QString& user_token) {
  user_token_ = user_token;
  ui_->check_pin->hide();
  ui_->sign_in_status->clear();
  UpdateSignedInState();
  authenticator_->FetchServers(user_token_);
}

void PlexSettingsPage::ServersFound(const QList<PlexServer>& servers) {
  const int index = ui_->server->currentIndex();
  const QString selected = index >= 0 && index < servers_.size()
                               ? servers_[index].machine_id
                               : QString();

  servers_ = servers;
  ui_->server->clear();
  for (const PlexServer& server : servers_) {
    ui_->server->addItem(server.name);
    if (server.machine_id == selected) {
      ui_->server->setCurrentIndex(ui_->server->count() - 1);
    }
  }

  if (servers_.isEmpty()) {
    ui_->login_state->SetAccountTypeText(
        tr("No Plex Media Servers were found on your account."));
    ui_->login_state->SetAccountTypeVisible(true);
  }
}

void PlexSettingsPage::AuthFailed(const QString& message) {
  ui_->sign_in_status->setText(message);
  UpdateSignedInState();
}

void PlexSettingsPage::UpdateSignedInState() {
  const bool signed_in = !user_token_.isEmpty();
  ui_->login_state->SetLoggedIn(signed_in ? LoginStateWidget::LoggedIn
                                          : LoginStateWidget::LoggedOut);
  ui_->server_group->setEnabled(signed_in);
}

void PlexSettingsPage::LoginStateChanged(PlexService::LoginState state) {
  QString text;
  switch (state) {
    case PlexService::LoginState_LoggedIn:
      text = tr("Connected to %1.").arg(service_->server_name());
      break;
    case PlexService::LoginState_Connecting:
      text = tr("Connecting to %1...").arg(service_->server_name());
      break;
    case PlexService::LoginState_ServerUnreachable:
      text = tr("Couldn't reach %1. Check that the server is running and "
                "that this computer can reach it.")
                 .arg(service_->server_name());
      break;
    case PlexService::LoginState_BadToken:
      text = tr("%1 refused the sign-in. Try signing out and in again.")
                 .arg(service_->server_name());
      break;
    case PlexService::LoginState_NotConfigured:
      break;
  }

  ui_->login_state->SetAccountTypeText(text);
  ui_->login_state->SetAccountTypeVisible(!text.isEmpty());
}
