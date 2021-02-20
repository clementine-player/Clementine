/* This file is part of Clementine.
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2011-2013, David Sansome <me@davidsansome.com>
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

#include "spotifysettingspage.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>

#include "config.h"
#include "core/network.h"
#include "internet/core/internetmodel.h"
#include "spotifymessages.pb.h"
#include "spotifyservice.h"
#include "ui/iconloader.h"
#include "ui_spotifysettingspage.h"

SpotifySettingsPage::SpotifySettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_SpotifySettingsPage),
      service_(InternetModel::Service<SpotifyService>()),
      validated_(false) {
  ui_->setupUi(this);

  setWindowIcon(IconLoader::Load("spotify", IconLoader::Provider));

  QFont bold_font(font());
  bold_font.setBold(true);
  ui_->blob_status->setFont(bold_font);

  connect(ui_->download_blob, SIGNAL(clicked()), SLOT(DownloadBlob()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(Login()));

  connect(service_, SIGNAL(LoginFinished(bool)), SLOT(LoginFinished(bool)));
  connect(service_, SIGNAL(BlobStateChanged()), SLOT(BlobStateChanged()));

  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->account_group);

  ui_->bitrate->addItem("96 " + tr("kbps"), cpb::spotify::Bitrate96k);
  ui_->bitrate->addItem("160 " + tr("kbps"), cpb::spotify::Bitrate160k);
  ui_->bitrate->addItem("320 " + tr("kbps"), cpb::spotify::Bitrate320k);

  BlobStateChanged();
}

SpotifySettingsPage::~SpotifySettingsPage() { delete ui_; }

void SpotifySettingsPage::BlobStateChanged() {
  const bool installed = service_->IsBlobInstalled();

  ui_->account_group->setEnabled(installed);
  ui_->blob_status->setText(installed ? tr("Installed") : tr("Not installed"));

#ifdef HAVE_SPOTIFY_DOWNLOADER
  ui_->download_blob->setEnabled(!installed);
#else
  ui_->download_blob->hide();
#endif
}

void SpotifySettingsPage::DownloadBlob() { service_->InstallBlob(); }

void SpotifySettingsPage::Login() {
  if (!service_->IsBlobInstalled()) {
    return;
  }

  if (ui_->username->text() == original_username_ &&
      ui_->password->text() == original_password_ &&
      service_->login_state() == SpotifyService::LoginState_LoggedIn) {
    return;
  }

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
  service_->Login(ui_->username->text(), ui_->password->text());
}

void SpotifySettingsPage::Load() {
  QSettings s;
  s.beginGroup(SpotifyService::kSettingsGroup);

  original_username_ = s.value("username").toString();

  ui_->username->setText(original_username_);
  validated_ = false;

  ui_->bitrate->setCurrentIndex(ui_->bitrate->findData(
      s.value("bitrate", cpb::spotify::Bitrate320k).toInt()));
  ui_->volume_normalisation->setChecked(
      s.value("volume_normalisation", false).toBool());

  UpdateLoginState();
}

void SpotifySettingsPage::Save() {
  QSettings s;
  s.beginGroup(SpotifyService::kSettingsGroup);

  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());

  s.setValue("bitrate",
             ui_->bitrate->itemData(ui_->bitrate->currentIndex()).toInt());
  s.setValue("volume_normalisation", ui_->volume_normalisation->isChecked());
}

void SpotifySettingsPage::LoginFinished(bool success) {
  validated_ = success;

  Save();
  UpdateLoginState();
}

void SpotifySettingsPage::UpdateLoginState() {
  const bool logged_in =
      service_->login_state() == SpotifyService::LoginState_LoggedIn;

  ui_->login_state->SetLoggedIn(
      logged_in ? LoginStateWidget::LoggedIn : LoginStateWidget::LoggedOut,
      ui_->username->text());
  ui_->login_state->SetAccountTypeVisible(!logged_in);

  switch (service_->login_state()) {
    case SpotifyService::LoginState_NoPremium:
      ui_->login_state->SetAccountTypeText(
          tr("You do not have a Spotify Premium account."));
      break;

    case SpotifyService::LoginState_Banned:
    case SpotifyService::LoginState_BadCredentials:
      ui_->login_state->SetAccountTypeText(
          tr("Your username or password was incorrect."));
      break;

    case SpotifyService::LoginState_ReloginFailed:
      ui_->login_state->SetAccountTypeText(
          tr("You have been logged out of Spotify, please re-enter your "
             "password."));
      break;

    default:
      ui_->login_state->SetAccountTypeText(
          tr("A Spotify Premium account is required."));
      break;
  }
}

void SpotifySettingsPage::Logout() {
  service_->Logout();
  UpdateLoginState();

  ui_->username->clear();
}
