/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "dropboxsettingspage.h"

#include "core/application.h"
#include "internet/core/internetmodel.h"
#include "internet/dropbox/dropboxservice.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_dropboxsettingspage.h"

DropboxSettingsPage::DropboxSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::DropboxSettingsPage),
      service_(dialog()->app()->internet_model()->Service<DropboxService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("dropbox", IconLoader::Provider));

  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));

  dialog()->installEventFilter(this);
}

DropboxSettingsPage::~DropboxSettingsPage() { delete ui_; }

void DropboxSettingsPage::Load() {
  if (service_->has_credentials()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
  }
}

void DropboxSettingsPage::Save() {
  QSettings s;
  s.beginGroup(DropboxService::kSettingsGroup);
}

void DropboxSettingsPage::LoginClicked() {
  service_->Login();

  ui_->login_button->setEnabled(false);
}

bool DropboxSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void DropboxSettingsPage::LogoutClicked() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  service_->ForgetCredentials();
}

void DropboxSettingsPage::Connected() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
}
