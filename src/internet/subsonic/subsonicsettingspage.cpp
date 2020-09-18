/* This file is part of Clementine.
   Copyright 2011, 2013, Alan Briolat <alan.briolat@gmail.com>
   Copyright 2013, Ross Wolfson <ross.wolfson@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "subsonicsettingspage.h"

#include <QSettings>

#include "core/logging.h"
#include "internet/core/internetmodel.h"
#include "ui/iconloader.h"
#include "ui_subsonicsettingspage.h"

SubsonicSettingsPage::SubsonicSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_SubsonicSettingsPage),
      service_(InternetModel::Service<SubsonicService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("subsonic", IconLoader::Provider));

  connect(ui_->server, SIGNAL(editingFinished()),
          SLOT(ServerEditingFinished()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));
  connect(service_, SIGNAL(LoginStateChanged(SubsonicService::LoginState)),
          SLOT(LoginStateChanged(SubsonicService::LoginState)));

  ui_->login_state->AddCredentialField(ui_->server);
  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialField(ui_->usesslv3);
  ui_->login_state->AddCredentialField(ui_->verifycert);
  ui_->login_state->AddCredentialGroup(ui_->server_group);

  ui_->login_state->SetAccountTypeText(
      tr("Streaming from a Subsonic server requires a valid server license "
         "after the 30-day trial period."));
  ui_->login_state->SetAccountTypeVisible(true);
}

SubsonicSettingsPage::~SubsonicSettingsPage() { delete ui_; }

void SubsonicSettingsPage::Load() {
  QSettings s;
  s.beginGroup(SubsonicService::kSettingsGroup);

  ui_->server->setText(s.value("server").toString());
  ui_->username->setText(s.value("username").toString());
  ui_->password->setText(s.value("password").toString());
  ui_->usesslv3->setChecked(s.value("usesslv3").toBool());
  ui_->verifycert->setChecked(s.value("verifycert", true).toBool());

  // If the settings are complete, SubsonicService will have used them already
  // and
  // we can tell the user if they worked
  if (ui_->server->text() != "" && ui_->username->text() != "") {
    LoginStateChanged(service_->login_state());
  }
}

void SubsonicSettingsPage::Save() {
  QSettings s;
  s.beginGroup(SubsonicService::kSettingsGroup);

  s.setValue("server", ui_->server->text());
  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());
  s.setValue("usesslv3", ui_->usesslv3->isChecked());
  s.setValue("verifycert", ui_->verifycert->isChecked());
}

void SubsonicSettingsPage::LoginStateChanged(
    SubsonicService::LoginState newstate) {
  const bool logged_in = newstate == SubsonicService::LoginState_Loggedin;

  ui_->login_state->SetLoggedIn(
      logged_in ? LoginStateWidget::LoggedIn : LoginStateWidget::LoggedOut,
      QString("%1 (%2)").arg(ui_->username->text()).arg(ui_->server->text()));
  ui_->login_state->SetAccountTypeVisible(!logged_in);

  switch (newstate) {
    case SubsonicService::LoginState_BadServer:
      ui_->login_state->SetAccountTypeText(
          tr("Could not connect to Subsonic, check server URL. "
             "Example: http://localhost:4040/"));
      break;

    case SubsonicService::LoginState_BadCredentials:
      ui_->login_state->SetAccountTypeText(tr("Wrong username or password."));
      break;

    case SubsonicService::LoginState_OutdatedClient:
      ui_->login_state->SetAccountTypeText(tr(
          "Incompatible Subsonic REST protocol version. Client must upgrade."));
      break;

    case SubsonicService::LoginState_OutdatedServer:
      ui_->login_state->SetAccountTypeText(tr(
          "Incompatible Subsonic REST protocol version. Server must upgrade."));
      break;

    case SubsonicService::LoginState_Unlicensed:
      ui_->login_state->SetAccountTypeText(
          tr("The trial period for the Subsonic server is over. "
             "Please donate to get a license key. Visit subsonic.org for "
             "details."));
      break;

    case SubsonicService::LoginState_OtherError:
      ui_->login_state->SetAccountTypeText(
          tr("An unspecified error occurred."));
      break;

    case SubsonicService::LoginState_ConnectionRefused:
      ui_->login_state->SetAccountTypeText(
          tr("Connection refused by server, check server URL. "
             "Example: http://localhost:4040/"));
      break;

    case SubsonicService::LoginState_HostNotFound:
      ui_->login_state->SetAccountTypeText(
          tr("Host not found, check server URL. "
             "Example: http://localhost:4040/"));
      break;

    case SubsonicService::LoginState_Timeout:
      ui_->login_state->SetAccountTypeText(
          tr("Connection timed out, check server URL. "
             "Example: http://localhost:4040/"));
      break;

    case SubsonicService::LoginState_SslError:
      ui_->login_state->SetAccountTypeText(
          tr("SSL handshake error, verify server configuration. "
             "SSLv3 option below may workaround some issues."));
      break;

    case SubsonicService::LoginState_IncompleteCredentials:
      ui_->login_state->SetAccountTypeText(tr(
          "Incomplete configuration, please ensure all fields are populated."));
      break;

    case SubsonicService::LoginState_RedirectLimitExceeded:
      ui_->login_state->SetAccountTypeText(
          tr("Redirect limit exceeded, verify server configuration."));
      break;

    case SubsonicService::LoginState_RedirectNoUrl:
      ui_->login_state->SetAccountTypeText(
          tr("HTTP 3xx status code received without URL, "
             "verify server configuration."));
      break;

    default:
      break;
  }
}

void SubsonicSettingsPage::ServerEditingFinished() {
  QString input = ui_->server->text();
  QUrl url = QUrl::fromUserInput(input);

  // Veto things that don't get guessed as an HTTP URL, the result will be
  // unhelpful
  if (!url.scheme().startsWith("http")) {
    return;
  }

  // If the user specified a /rest location, remove it since we're going to
  // re-add it later
  url = SubsonicService::ScrubUrl(url);

  ui_->server->setText(url.toString());
  qLog(Debug) << "URL fixed:" << input << "to" << url;
}

void SubsonicSettingsPage::Login() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
  service_->Login(ui_->server->text(), ui_->username->text(),
                  ui_->password->text(), ui_->usesslv3->isChecked(),
                  ui_->verifycert->isChecked());
}

void SubsonicSettingsPage::Logout() {
  ui_->username->setText("");
  ui_->password->setText("");
}
