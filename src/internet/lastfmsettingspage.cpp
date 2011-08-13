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

// Use Qt specific icons, since freedesktop doesn't seem to have suitable icons.
const char* kSubscribedIcon = "task-complete";
const char* kNotSubscribedIcon = "dialog-warning";
const char* kWaitingIcon = ":spinner.gif";

LastFMSettingsPage::LastFMSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    service_(static_cast<LastFMService*>(InternetModel::ServiceByName("Last.fm"))),
    ui_(new Ui_LastFMSettingsPage),
    loading_icon_(new QMovie(kWaitingIcon, QByteArray(), this)),
    waiting_for_auth_(false)
{
  ui_->setupUi(this);
  ui_->busy->hide();

  // Icons
  setWindowIcon(QIcon(":/last.fm/as.png"));
  ui_->sign_out->setIcon(IconLoader::Load("list-remove"));
  ui_->warn_icon->setPixmap(IconLoader::Load("dialog-warning").pixmap(16));
  ui_->warn_icon->setMinimumSize(16, 16);

  connect(service_, SIGNAL(AuthenticationComplete(bool)), SLOT(AuthenticationComplete(bool)));
  connect(service_, SIGNAL(UpdatedSubscriberStatus(bool)), SLOT(UpdatedSubscriberStatus(bool)));
  connect(ui_->sign_out, SIGNAL(clicked()), SLOT(SignOut()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));

  ui_->username->setMinimumWidth(QFontMetrics(QFont()).width("WWWWWWWWWWWW"));
  resize(sizeHint());
}

LastFMSettingsPage::~LastFMSettingsPage() {
  delete ui_;
}

void LastFMSettingsPage::Login() {
  ui_->busy->show();
  waiting_for_auth_ = true;

  service_->Authenticate(ui_->username->text(), ui_->password->text());
}

void LastFMSettingsPage::AuthenticationComplete(bool success) {
  if (!waiting_for_auth_)
    return; // Wasn't us that was waiting for auth

  ui_->busy->hide();
  waiting_for_auth_ = false;

  if (success) {
    // Clear password just to be sure
    ui_->password->clear();
    // Save settings
    Save();
    RefreshControls(true);
  } else {
    QMessageBox::warning(this, tr("Authentication failed"), tr("Your Last.fm credentials were incorrect"));
  }

  service_->UpdateSubscriberStatus();
}

void LastFMSettingsPage::Load() {
  ui_->scrobble->setChecked(service_->IsScrobblingEnabled());
  ui_->love_ban_->setChecked(service_->AreButtonsVisible());
  ui_->scrobble_button->setChecked(service_->IsScrobbleButtonVisible());

  ui_->icon->setMovie(loading_icon_);
  loading_icon_->start();
  if (service_->IsAuthenticated()) {
    service_->UpdateSubscriberStatus();
  }

  RefreshControls(service_->IsAuthenticated());
}

void LastFMSettingsPage::UpdatedSubscriberStatus(bool is_subscriber) {
  const char* icon_path = is_subscriber ? kSubscribedIcon : kNotSubscribedIcon;
  ui_->icon->setPixmap(IconLoader::Load(icon_path).pixmap(16));
  loading_icon_->stop();

  if (is_subscriber) {
    ui_->subscriber_warning->hide();
    ui_->warn_icon->hide();
  } else {
    ui_->warn_icon->show();
    if (service_->HasConnectionProblems()) {
      ui_->subscriber_warning->setText(
        tr("Clementine couldn't fetch your subscription status since there are problems "
           "with your connection. Played tracks will be cached and sent later to Last.fm."));
    } else {
      ui_->subscriber_warning->setText(
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
  s.endGroup();

  service_->ReloadSettings();
}

void LastFMSettingsPage::SignOut() {
  ui_->username->clear();
  ui_->password->clear();
  RefreshControls(false);

  service_->SignOut();
}

void LastFMSettingsPage::RefreshControls(bool authenticated) {
  ui_->groupBox->setVisible(!authenticated);
  ui_->sign_out->setVisible(authenticated);
  if (authenticated) {
    ui_->status->setText(QString(tr("You're logged in as <b>%1</b>")).arg(lastfm::ws::Username));
  } else {
    ui_->icon->setPixmap(IconLoader::Load("task-reject").pixmap(16));
    ui_->status->setText(tr("Please fill in the blanks to login into Last.fm"));

    ui_->subscriber_warning->setText(
        tr("You can scrobble tracks for free, but only "
           "<span style=\" font-weight:600;\">paid subscribers</span> "
           "can stream Last.fm radio from Clementine."));
    ui_->subscriber_warning->show();
    ui_->warn_icon->show();
  }
}
