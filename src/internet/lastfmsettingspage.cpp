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

#include "lastfmsettingspage.h"
#include "lastfmservice.h"
#include "internetmodel.h"
#include "ui_lastfmsettingspage.h"
#include "ui/iconloader.h"

#include <lastfm/ws.h>

#include <QMessageBox>
#include <QMovie>
#include <QSettings>

LastFMSettingsPage::LastFMSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      service_(
          static_cast<LastFMService*>(InternetModel::ServiceByName("Last.fm"))),
      ui_(new Ui_LastFMSettingsPage),
      waiting_for_auth_(false) {
  ui_->setupUi(this);

  // Icons
  setWindowIcon(QIcon(":/last.fm/as.png"));

  connect(service_, SIGNAL(AuthenticationComplete(bool, QString)),
          SLOT(AuthenticationComplete(bool, QString)));
  connect(service_, SIGNAL(UpdatedSubscriberStatus(bool)),
          SLOT(UpdatedSubscriberStatus(bool)));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(Login()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));

  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->groupBox);

  ui_->username->setMinimumWidth(QFontMetrics(QFont()).width("WWWWWWWWWWWW"));
  resize(sizeHint());
}

LastFMSettingsPage::~LastFMSettingsPage() { delete ui_; }

void LastFMSettingsPage::Login() {
  waiting_for_auth_ = true;

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);
  service_->Authenticate(ui_->username->text(), ui_->password->text());
}

void LastFMSettingsPage::AuthenticationComplete(bool success,
                                                const QString& message) {
  if (!waiting_for_auth_) return;  // Wasn't us that was waiting for auth

  waiting_for_auth_ = false;

  if (success) {
    // Clear password just to be sure
    ui_->password->clear();
    // Save settings
    Save();
  } else {
    QString dialog_text = tr("Your Last.fm credentials were incorrect");
    if (!message.isEmpty()) {
      dialog_text = message;
    }
    QMessageBox::warning(this, tr("Authentication failed"), dialog_text);
  }

  RefreshControls(success);
  service_->UpdateSubscriberStatus();
}

void LastFMSettingsPage::Load() {
  ui_->scrobble->setChecked(service_->IsScrobblingEnabled());
  ui_->love_ban_->setChecked(service_->AreButtonsVisible());
  ui_->scrobble_button->setChecked(service_->IsScrobbleButtonVisible());
  ui_->prefer_albumartist->setChecked(service_->PreferAlbumArtist());

  if (service_->IsAuthenticated()) {
    service_->UpdateSubscriberStatus();
  }

  RefreshControls(service_->IsAuthenticated());
}

void LastFMSettingsPage::UpdatedSubscriberStatus(bool is_subscriber) {
  ui_->login_state->SetAccountTypeVisible(!is_subscriber);

  if (!is_subscriber) {
    if (service_->HasConnectionProblems()) {
      ui_->login_state->SetAccountTypeText(
          tr("Clementine couldn't fetch your subscription status since there "
             "are problems "
             "with your connection. Played tracks will be cached and sent "
             "later to Last.fm."));
    } else {
      ui_->login_state->SetAccountTypeText(
          tr("You will not be able to play Last.fm radio stations "
             "as you are not a Last.fm subscriber."));
    }
  }
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
  ui_->username->clear();
  ui_->password->clear();
  RefreshControls(false);

  service_->SignOut();
}

void LastFMSettingsPage::RefreshControls(bool authenticated) {
  ui_->login_state->SetLoggedIn(
      authenticated ? LoginStateWidget::LoggedIn : LoginStateWidget::LoggedOut,
      lastfm::ws::Username);
  ui_->login_state->SetAccountTypeVisible(!authenticated);

  if (!authenticated) {
    ui_->login_state->SetAccountTypeText(
        tr("You can scrobble tracks for free, but only "
           "<span style=\" font-weight:600;\">paid subscribers</span> "
           "can stream Last.fm radio from Clementine."));
  }
}
