/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "digitallyimportedservicebase.h"
#include "digitallyimportedsettingspage.h"
#include "ui_digitallyimportedsettingspage.h"

#include <QSettings>


DigitallyImportedSettingsPage::DigitallyImportedSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_DigitallyImportedSettingsPage)
{
  ui_->setupUi(this);
  setWindowIcon(QIcon(":/providers/digitallyimported-32.png"));

  ui_->login_state->SetAccountTypeText(tr(
      "You can listen for free without an account, but Premium members can "
      "listen to higher quality streams without advertisements."));
  ui_->login_state->SetAccountTypeVisible(true);
  ui_->login_state->HideLoggedInState();
}

DigitallyImportedSettingsPage::~DigitallyImportedSettingsPage() {
  delete ui_;
}

void DigitallyImportedSettingsPage::Load() {
  QSettings s;
  s.beginGroup(DigitallyImportedServiceBase::kSettingsGroup);

  ui_->audio_type->setCurrentIndex(s.value("audio_type", 0).toInt());
  ui_->username->setText(s.value("username").toString());
  ui_->password->setText(s.value("password").toString());
}

void DigitallyImportedSettingsPage::Save() {
  QSettings s;
  s.beginGroup(DigitallyImportedServiceBase::kSettingsGroup);

  s.setValue("audio_type", ui_->audio_type->currentIndex());
  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());
}


