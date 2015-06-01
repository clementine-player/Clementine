/* This file is part of Clementine.
   Copyright 2015, Marco Kirchner <kirchnermarco@gmail.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#include "playmusicservice.h"
#include "playmusicsettingspage.h"
#include "ui_playmusicsettingspage.h"
#include "core/application.h"

PlayMusicSettingsPage::PlayMusicSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::PlayMusicSettingsPage),
      service_(
          dialog()->app()->internet_model()->Service<PlayMusicService>()) {
  ui_->setupUi(this);
  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(LoginStateChanged()), SLOT(LoginStateChanged()));
}

PlayMusicSettingsPage::~PlayMusicSettingsPage() { delete ui_; }

void PlayMusicSettingsPage::Load() {
    QSettings s;
    s.beginGroup(PlayMusicService::kSettingsGroup);
    ui_->username->setText(s.value("username").toString());
    LoginStateChanged();
}

void PlayMusicSettingsPage::Save() {
    QSettings s;
    s.beginGroup(PlayMusicService::kSettingsGroup);
    s.setValue("username", ui_->username->text());
}

void PlayMusicSettingsPage::LoginClicked() {
  service_->Login(ui_->username->text(), ui_->password->text());
  ui_->login->setEnabled(false);
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
}

void PlayMusicSettingsPage::LogoutClicked() {
  service_->Logout();
  ui_->login->setEnabled(true);
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void PlayMusicSettingsPage::LoginStateChanged() {
    SetLoggedIn(service_->IsLoggedIn());
}

void PlayMusicSettingsPage::SetLoggedIn(bool loggedIn)
{
    ui_->login->setEnabled(!loggedIn);
    ui_->login_state->SetLoggedIn(loggedIn ? LoginStateWidget::LoggedIn : LoginStateWidget::LoggedOut);
    ui_->username->setEnabled(!loggedIn);
    ui_->password->setEnabled(!loggedIn);
}
