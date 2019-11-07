/* This file is part of Clementine.
   Copyright 2009-2011, 2013, David Sansome <me@davidsansome.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2011, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012, Kacper "mattrick" Banasik <mattrick@jabster.pl>
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

#include "lastfmsettingspage.h"
#include "ui_lastfmsettingspage.h"

#include <lastfm5/ws.h>

#include <QDesktopServices>
#include <QMessageBox>
#include <QSettings>

#include "lastfmservice.h"
#include "internet/core/internetmodel.h"
#include "core/application.h"
#include "ui/iconloader.h"

LastFMSettingsPage::LastFMSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      service_(static_cast<LastFMService*>(dialog->app()->scrobbler())),
      ui_(new Ui_LastFMSettingsPage),
      waiting_for_auth_(false) {
  ui_->setupUi(this);

  // Icons
  setWindowIcon(IconLoader::Load("lastfm", IconLoader::Provider));

  connect(service_, SIGNAL(AuthenticationComplete(bool)),
          SLOT(AuthenticationComplete(bool)));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(Login()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));

  ui_->login_state->AddCredentialGroup(ui_->login_container);

  resize(sizeHint());
}

LastFMSettingsPage::~LastFMSettingsPage() { delete ui_; }

void LastFMSettingsPage::Login() {
  waiting_for_auth_ = true;

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
  service_->Authenticate();
}

void LastFMSettingsPage::AuthenticationComplete(bool success) {
  if (!waiting_for_auth_) return;  // Wasn't us that was waiting for auth

  waiting_for_auth_ = false;

  if (success) {
    // Save settings
    Save();
  } else {
    QString dialog_text = tr("Could not log in to Last.fm. Please try again.");
    QMessageBox::warning(this, tr("Authentication failed"), dialog_text);
  }

  RefreshControls(success);
}

void LastFMSettingsPage::Load() {
  ui_->scrobble->setChecked(service_->IsScrobblingEnabled());
  ui_->love_ban_->setChecked(service_->AreButtonsVisible());
  ui_->scrobble_button->setChecked(service_->IsScrobbleButtonVisible());
  ui_->prefer_albumartist->setChecked(service_->PreferAlbumArtist());

  RefreshControls(service_->IsAuthenticated());
}

void LastFMSettingsPage::Save() {
  QSettings s;
  s.beginGroup(LastFMService::kSettingsGroup);
  s.setValue("ScrobblingEnabled", ui_->scrobble->isChecked());
  s.setValue("ShowLoveBanButtons", ui_->love_ban_->isChecked());
  s.setValue("ShowScrobbleButton", ui_->scrobble_button->isChecked());
  s.setValue("PreferAlbumArtist", ui_->prefer_albumartist->isChecked());
  s.endGroup();

  service_->ReloadSettings();
}

void LastFMSettingsPage::Logout() {
  RefreshControls(false);

  service_->SignOut();
}

void LastFMSettingsPage::RefreshControls(bool authenticated) {
  ui_->login_state->SetLoggedIn(
      authenticated ? LoginStateWidget::LoggedIn : LoginStateWidget::LoggedOut,
      lastfm::ws::Username);
}
