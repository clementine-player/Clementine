/* This file is part of Clementine.
   Copyright 2011, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011, David Sansome <me@davidsansome.com>
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

#include "groovesharkservice.h"
#include "groovesharksettingspage.h"
#include "internet/core/internetmodel.h"
#include "core/logging.h"
#include "core/network.h"
#include "ui_groovesharksettingspage.h"
#include "ui/iconloader.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>

GroovesharkSettingsPage::GroovesharkSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_GroovesharkSettingsPage),
      service_(InternetModel::Service<GroovesharkService>()),
      validated_(false) {
  ui_->setupUi(this);

  setWindowIcon(QIcon(":/providers/grooveshark.png"));

  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(Login()));

  connect(service_, SIGNAL(LoginFinished(bool)), SLOT(LoginFinished(bool)));

  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->account_group);
}

GroovesharkSettingsPage::~GroovesharkSettingsPage() { delete ui_; }

void GroovesharkSettingsPage::Login() {
  if (service_->IsLoggedIn()) {
    return;
  }

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
  service_->Login(ui_->username->text(), ui_->password->text());
}

void GroovesharkSettingsPage::Load() {
  QSettings s;
  s.beginGroup(GroovesharkService::kSettingsGroup);

  original_username_ = s.value("username").toString();

  ui_->username->setText(original_username_);
  validated_ = false;

  UpdateLoginState();

  ui_->sort_alphabetically->setChecked(s.value("sort_alphabetically").toBool());
}

void GroovesharkSettingsPage::Save() {
  QSettings s;
  s.beginGroup(GroovesharkService::kSettingsGroup);

  s.setValue("username", ui_->username->text());
  s.setValue("sessionid", service_->session_id());
  const bool old_sort_value = s.value("sort_alphabetically").toBool();
  const bool new_sort_value = ui_->sort_alphabetically->isChecked();
  if (old_sort_value != new_sort_value) {
    s.setValue("sort_alphabetically", new_sort_value);
    service_->RefreshItems();
  }
}

void GroovesharkSettingsPage::LoginFinished(bool success) {
  validated_ = success;

  Save();
  UpdateLoginState();
}

void GroovesharkSettingsPage::UpdateLoginState() {
  const bool logged_in = service_->IsLoggedIn();

  ui_->login_state->SetLoggedIn(
      logged_in ? LoginStateWidget::LoggedIn : LoginStateWidget::LoggedOut,
      ui_->username->text());
  ui_->login_state->SetAccountTypeVisible(!logged_in);

  switch (service_->login_state()) {
    case GroovesharkService::LoginState_NoPremium:
      ui_->login_state->SetAccountTypeText(
          tr("You do not have a Grooveshark Anywhere account."));
      break;

    case GroovesharkService::LoginState_AuthFailed:
      ui_->login_state->SetAccountTypeText(
          tr("Your username or password was incorrect."));
      break;

    default:
      ui_->login_state->SetAccountTypeText(
          tr("A Grooveshark Anywhere account is required."));
      break;
  }
}

void GroovesharkSettingsPage::Logout() {
  service_->Logout();
  UpdateLoginState();
}
