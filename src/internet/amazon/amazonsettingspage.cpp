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

#include "amazonsettingspage.h"
#include "ui_amazonsettingspage.h"

#include "core/application.h"
#include "internet/amazon/amazonclouddrive.h"
#include "internet/core/internetmodel.h"
#include "ui/settingsdialog.h"

AmazonSettingsPage::AmazonSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::AmazonSettingsPage),
      service_(dialog()->app()->internet_model()->Service<AmazonCloudDrive>()) {
  ui_->setupUi(this);
  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));

  dialog()->installEventFilter(this);
}

AmazonSettingsPage::~AmazonSettingsPage() { delete ui_; }

void AmazonSettingsPage::Load() {
  QSettings s;
  s.beginGroup(AmazonCloudDrive::kSettingsGroup);

  const QString token = s.value("refresh_token").toString();

  if (!token.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
  }
}

void AmazonSettingsPage::Save() {
  QSettings s;
  s.beginGroup(AmazonCloudDrive::kSettingsGroup);
}

void AmazonSettingsPage::LoginClicked() {
  service_->Connect();
  ui_->login_button->setEnabled(false);
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
}

bool AmazonSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void AmazonSettingsPage::LogoutClicked() {
  service_->ForgetCredentials();
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void AmazonSettingsPage::Connected() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn);
}
