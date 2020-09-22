/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
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

#include "internet/podcasts/podcastsettingspage.h"

#include <QFileDialog>
#include <QSettings>

#include "core/application.h"
#include "core/timeconstants.h"
#include "gpoddersync.h"
#include "library/librarydirectorymodel.h"
#include "library/librarymodel.h"
#include "podcastdownloader.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_podcastsettingspage.h"

const char* PodcastSettingsPage::kSettingsGroup = "Podcasts";

PodcastSettingsPage::PodcastSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_PodcastSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("podcast", IconLoader::Provider));

  connect(ui_->login, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(ui_->download_dir_browse, SIGNAL(clicked()),
          SLOT(DownloadDirBrowse()));

  GPodderSync* gsync = dialog->app()->gpodder_sync();
  connect(gsync, SIGNAL(LoginSuccess()), SLOT(GpodderLoginSuccess()));
  connect(gsync, SIGNAL(LoginFailure(const QString&)),
          SLOT(GpodderLoginFailure(const QString&)));

  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->device_name);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->login_group);

  ui_->check_interval->setItemData(0, 0);             // manually
  ui_->check_interval->setItemData(1, 10 * 60);       // 10 minutes
  ui_->check_interval->setItemData(2, 20 * 60);       // 20 minutes
  ui_->check_interval->setItemData(3, 30 * 60);       // 30 minutes
  ui_->check_interval->setItemData(4, 60 * 60);       // 1 hour
  ui_->check_interval->setItemData(5, 2 * 60 * 60);   // 2 hours
  ui_->check_interval->setItemData(6, 6 * 60 * 60);   // 6 hours
  ui_->check_interval->setItemData(7, 12 * 60 * 60);  // 12 hours
}

PodcastSettingsPage::~PodcastSettingsPage() { delete ui_; }

void PodcastSettingsPage::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  const int update_interval = s.value("update_interval_secs", 0).toInt();
  ui_->check_interval->setCurrentIndex(
      ui_->check_interval->findData(update_interval));

  const QString default_download_dir =
      dialog()->app()->podcast_downloader()->DefaultDownloadDir();
  ui_->download_dir->setText(QDir::toNativeSeparators(
      s.value("download_dir", default_download_dir).toString()));

  ui_->auto_download->setChecked(s.value("auto_download", false).toBool());
  ui_->hide_listened->setChecked(s.value("hide_listened", false).toBool());
  ui_->delete_after->setValue(s.value("delete_after", 0).toInt() / kSecsPerDay);
  ui_->show_episodes->setValue(s.value("show_episodes", 0).toInt());
  ui_->username->setText(s.value("gpodder_username").toString());
  ui_->device_name->setText(
      s.value("gpodder_device_name", GPodderSync::DefaultDeviceName())
          .toString());

  if (dialog()->app()->gpodder_sync()->is_logged_in()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn,
                                  ui_->username->text());
  } else {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  }
}

void PodcastSettingsPage::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("update_interval_secs", ui_->check_interval->itemData(
                                         ui_->check_interval->currentIndex()));
  s.setValue("download_dir",
             QDir::fromNativeSeparators(ui_->download_dir->text()));
  s.setValue("auto_download", ui_->auto_download->isChecked());
  s.setValue("hide_listened", ui_->hide_listened->isChecked());
  s.setValue("delete_after", ui_->delete_after->value() * kSecsPerDay);
  s.setValue("show_episodes", ui_->show_episodes->value());
  s.setValue("gpodder_device_name", ui_->device_name->text());
}

void PodcastSettingsPage::LoginClicked() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);

  dialog()->app()->gpodder_sync()->Login(
      ui_->username->text(), ui_->password->text(), ui_->device_name->text());
}

void PodcastSettingsPage::GpodderLoginSuccess() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn,
                                ui_->username->text());
  ui_->login_state->SetAccountTypeVisible(false);
}

void PodcastSettingsPage::GpodderLoginFailure(const QString& error) {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut,
                                ui_->username->text());
  ui_->login_state->SetAccountTypeVisible(true);
  ui_->login_state->SetAccountTypeText(tr("Login failed") + ": " + error);
}

void PodcastSettingsPage::LogoutClicked() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  ui_->password->clear();
  dialog()->app()->gpodder_sync()->Logout();
}

void PodcastSettingsPage::DownloadDirBrowse() {
  QString directory = QFileDialog::getExistingDirectory(
      this, tr("Choose podcast download directory"), ui_->download_dir->text());
  if (directory.isEmpty()) return;

  ui_->download_dir->setText(QDir::toNativeSeparators(directory));
}
