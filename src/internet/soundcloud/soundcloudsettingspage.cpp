/* This file is part of Clementine.
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

#include "soundcloudservice.h"
#include "soundcloudsettingspage.h"
#include "ui_soundcloudsettingspage.h"
#include "core/application.h"
#include "internet/core/internetmodel.h"
#include "ui/iconloader.h"

SoundCloudSettingsPage::SoundCloudSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::SoundCloudSettingsPage),
      service_(
          dialog()->app()->internet_model()->Service<SoundCloudService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("soundcloud", IconLoader::provider));
  
  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));

  dialog()->installEventFilter(this);
}

SoundCloudSettingsPage::~SoundCloudSettingsPage() { delete ui_; }

void SoundCloudSettingsPage::Load() {
  if (service_->IsLoggedIn()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
  }
}

void SoundCloudSettingsPage::Save() {
  // Everything is done in the service: nothing to do here
}

void SoundCloudSettingsPage::LoginClicked() {
  service_->Connect();
  ui_->login_button->setEnabled(false);
}

bool SoundCloudSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void SoundCloudSettingsPage::LogoutClicked() {
  service_->Logout();
  ui_->login_button->setEnabled(true);
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void SoundCloudSettingsPage::Connected() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
}
