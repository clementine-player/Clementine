/* This file is part of Clementine.
   Copyright 2013, Vlad Maltsev <shedwardx@gmail.com>

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

#include "vksettingspage.h"

#include <QDir>
#include <QFileDialog>

#include "ui_vksettingspage.h"
#include "core/application.h"
#include "core/logging.h"
#include "internet/vkservice.h"

VkSettingsPage::VkSettingsPage(SettingsDialog *parent)
  : SettingsPage(parent),
    ui_(new Ui::VkSettingsPage),
    service_(dialog()->app()->internet_model()->Service<VkService>()) {
  ui_->setupUi(this);
  connect(service_, SIGNAL(LoginSuccess(bool)),
          SLOT(LoginSuccess(bool)));
  connect(ui_->choose_path, SIGNAL(clicked()),
          SLOT(CacheDirBrowse()));
  connect(ui_->reset, SIGNAL(clicked()),
          SLOT(ResetCasheFilenames()));
}

VkSettingsPage::~VkSettingsPage() {
  delete ui_;
}

void VkSettingsPage::Load() {
  service_->ReloadSettings();

  ui_->maxGlobalSearch->setValue(service_->maxGlobalSearch());
  ui_->enable_caching->setChecked(service_->isCachingEnabled());
  ui_->cache_dir->setText(service_->cacheDir());
  ui_->cache_filename->setText(service_->cacheFilename());
  ui_->love_button_is_add_to_mymusic->setChecked(service_->isLoveAddToMyMusic());
  ui_->groups_in_global_search->setChecked(service_->isGroupsInGlobalSearch());

  if (service_->HasAccount()) {
    LogoutWidgets();
  } else {
    LoginWidgets();
  }
}

void VkSettingsPage::Save() {
  QSettings s;
  s.beginGroup(VkService::kSettingGroup);

  s.setValue("max_global_search", ui_->maxGlobalSearch->value());
  s.setValue("cache_enabled", ui_->enable_caching->isChecked());
  s.setValue("cache_dir", ui_->cache_dir->text());
  s.setValue("cache_filename", ui_->cache_filename->text());
  s.setValue("love_is_add_to_my_music", ui_->love_button_is_add_to_mymusic->isChecked());
  s.setValue("groups_in_global_search", ui_->groups_in_global_search->isChecked());

  service_->ReloadSettings();
}

void VkSettingsPage::Login() {
  ui_->login_button->setEnabled(false);
  service_->Login();
}

void VkSettingsPage::LoginSuccess(bool success) {
  if (success) {
    LogoutWidgets();
  } else {
    LoginWidgets();
  }
}

void VkSettingsPage::Logout() {
  ui_->login_button->setEnabled(false);
  service_->Logout();
  LoginWidgets();
}

void VkSettingsPage::CacheDirBrowse() {
  QString directory = QFileDialog::getExistingDirectory(
    this, tr("Choose Vk.com cache directory"), ui_->cache_dir->text());
  if (directory.isEmpty()) {
    return;
  }

  ui_->cache_dir->setText(QDir::toNativeSeparators(directory));
}

void VkSettingsPage::ResetCasheFilenames() {
  ui_->cache_filename->setText(VkService::kDefCacheFilename);
}

void VkSettingsPage::LoginWidgets() {
  ui_->login_button->setText(tr("Login"));
  ui_->name->setText("");
  ui_->login_button->setEnabled(true);

  connect(ui_->login_button, SIGNAL(clicked()),
          SLOT(Login()), Qt::UniqueConnection);
  disconnect(ui_->login_button, SIGNAL(clicked()),
             this, SLOT(Logout()));
}

void VkSettingsPage::LogoutWidgets() {
  ui_->login_button->setText(tr("Logout"));
  ui_->name->setText(tr("Loading..."));
  ui_->login_button->setEnabled(true);

  connect(service_, SIGNAL(NameUpdated(QString)),
          ui_->name, SLOT(setText(QString)), Qt::UniqueConnection);
  service_->RequestUserProfile();

  connect(ui_->login_button, SIGNAL(clicked()),
          SLOT(Logout()), Qt::UniqueConnection);
  disconnect(ui_->login_button, SIGNAL(clicked()),
             this, SLOT(Login()));
}
