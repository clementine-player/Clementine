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

#include "magnatuneconfig.h"
#include "magnatuneservice.h"
#include "radiomodel.h"
#include "ui_magnatuneconfig.h"

#include <QSettings>

MagnatuneConfig::MagnatuneConfig(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_MagnatuneConfig)
{
  ui_->setupUi(this);

  connect(ui_->membership, SIGNAL(currentIndexChanged(int)), SLOT(MembershipChanged(int)));
}

MagnatuneConfig::~MagnatuneConfig() {
  delete ui_;
}

void MagnatuneConfig::Load() {
  QSettings s;
  s.beginGroup(MagnatuneService::kSettingsGroup);

  ui_->membership->setCurrentIndex(s.value("membership", MagnatuneService::Membership_None).toInt());
  ui_->username->setText(s.value("username").toString());
  ui_->password->setText(s.value("password").toString());
  ui_->format->setCurrentIndex(s.value("format", MagnatuneService::Format_Ogg).toInt());
}

void MagnatuneConfig::Save() {
  QSettings s;
  s.beginGroup(MagnatuneService::kSettingsGroup);

  s.setValue("membership", ui_->membership->currentIndex());
  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());
  s.setValue("format", ui_->format->currentIndex());

  RadioModel::Service<MagnatuneService>()->ReloadSettings();
}

void MagnatuneConfig::MembershipChanged(int value) {
  bool enabled = MagnatuneService::MembershipType(value) !=
                 MagnatuneService::Membership_None;
  ui_->login_container->setEnabled(enabled);
  ui_->preferences_group->setEnabled(enabled);
}
