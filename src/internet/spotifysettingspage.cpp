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

#include "spotifysettingspage.h"

#include "core/network.h"
#include "spotifyservice.h"
#include "internetmodel.h"
#include "ui_spotifysettingspage.h"
#include "ui/iconloader.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>

SpotifySettingsPage::SpotifySettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    network_(new NetworkAccessManager(this)),
    ui_(new Ui_SpotifySettingsPage),
    service_(InternetModel::Service<SpotifyService>()),
    validated_(false)
{
  ui_->setupUi(this);
  ui_->busy->hide();
  ui_->warn_icon->setPixmap(IconLoader::Load("dialog-warning").pixmap(16));

  setWindowIcon(QIcon(":/icons/svg/spotify.svg"));

  QFont bold_font(font());
  bold_font.setBold(true);
  ui_->blob_status->setFont(bold_font);

  connect(ui_->download_blob, SIGNAL(clicked()), SLOT(DownloadBlob()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));

  connect(service_, SIGNAL(LoginFinished(bool)), SLOT(LoginFinished(bool)));
  connect(service_, SIGNAL(BlobStateChanged()), SLOT(BlobStateChanged()));

  BlobStateChanged();
}

SpotifySettingsPage::~SpotifySettingsPage() {
  delete ui_;
}

void SpotifySettingsPage::BlobStateChanged() {
  const bool installed = service_->IsBlobInstalled();

  ui_->account_group->setEnabled(installed);
  ui_->blob_status->setText(installed ? tr("Installed") : tr("Not installed"));

#ifdef Q_OS_LINUX
  ui_->download_blob->setEnabled(!installed);
#else
  ui_->download_blob->setEnabled(false);
#endif
}

void SpotifySettingsPage::DownloadBlob() {
  service_->InstallBlob();
}

void SpotifySettingsPage::Login() {
  if (!service_->IsBlobInstalled()) {
    return;
  }

  if (ui_->username->text() == original_username_ &&
      ui_->password->text() == original_password_) {
    return;
  }

  if (!validated_) {
    return;
  }

  ui_->busy->show();
  service_->Login(ui_->username->text(), ui_->password->text());
}

void SpotifySettingsPage::Load() {
  QSettings s;
  s.beginGroup(SpotifyService::kSettingsGroup);

  original_username_ = s.value("username").toString();
  original_password_ = s.value("password").toString();

  ui_->username->setText(original_username_);
  ui_->password->setText(original_password_);
  validated_ = false;
}

void SpotifySettingsPage::Save() {
  QSettings s;
  s.beginGroup(SpotifyService::kSettingsGroup);

  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());

  InternetModel::Service<SpotifyService>()->ReloadSettings();
}

void SpotifySettingsPage::LoginFinished(bool success) {
  validated_ = success;
  ui_->busy->hide();
  ui_->login->setEnabled(!success);

  // Save the settings
  Save();
}
