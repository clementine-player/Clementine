/* This file is part of Clementine.
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#include "skydrivesettingspage.h"

#include <QSortFilterProxyModel>

#include "core/application.h"
#include "internet/core/internetmodel.h"
#include "internet/skydrive/skydriveservice.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_skydrivesettingspage.h"

SkydriveSettingsPage::SkydriveSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::SkydriveSettingsPage),
      service_(dialog()->app()->internet_model()->Service<SkydriveService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("skydrive", IconLoader::Provider));

  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));

  dialog()->installEventFilter(this);
}

SkydriveSettingsPage::~SkydriveSettingsPage() { delete ui_; }

void SkydriveSettingsPage::Load() {
  QSettings s;
  s.beginGroup(SkydriveService::kSettingsGroup);

  const QString name = s.value("name").toString();

  if (!name.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, name);
  }
}

void SkydriveSettingsPage::Save() {
  QSettings s;
  s.beginGroup(SkydriveService::kSettingsGroup);
}

void SkydriveSettingsPage::LoginClicked() {
  service_->Connect();
  ui_->login_button->setEnabled(false);
}

bool SkydriveSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void SkydriveSettingsPage::LogoutClicked() {
  service_->ForgetCredentials();
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
}

void SkydriveSettingsPage::Connected() {
  QSettings s;
  s.beginGroup(SkydriveService::kSettingsGroup);

  const QString name = s.value("name").toString();

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, name);
}
