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

#include "lastfmconfig.h"
#include "lastfmservice.h"
#include "radiomodel.h"
#include "ui_lastfmconfig.h"
#include "ui/iconloader.h"

#include <lastfm/ws.h>

#include <QMessageBox>
#include <QSettings>

// Use Qt specific icons, since freedesktop doesn't seem to have suitable icons.
const char* kSubscribedIcon = ":/trolltech/styles/commonstyle/images/standardbutton-apply-16.png";
const char* kNotSubscribedIcon = ":/trolltech/styles/commonstyle/images/standardbutton-no-16.png";

LastFMConfig::LastFMConfig(QWidget *parent)
  : QWidget(parent),
    service_(static_cast<LastFMService*>(RadioModel::ServiceByName("Last.fm"))),
    ui_(new Ui_LastFMConfig),
    waiting_for_auth_(false)
{
  ui_->setupUi(this);
  ui_->busy->hide();

  // Icons
  ui_->sign_out->setIcon(IconLoader::Load("list-remove"));
  ui_->label->setPixmap(IconLoader::Load("dialog-warning").pixmap(16));

  connect(service_, SIGNAL(AuthenticationComplete(bool)), SLOT(AuthenticationComplete(bool)));
  connect(ui_->sign_out, SIGNAL(clicked()), SLOT(SignOut()));

  ui_->username->setMinimumWidth(QFontMetrics(QFont()).width("WWWWWWWWWWWW"));
  resize(sizeHint());
}

LastFMConfig::~LastFMConfig() {
  delete ui_;
}

bool LastFMConfig::NeedsValidation() const {
  return !ui_->username->text().isEmpty() && !ui_->password->text().isEmpty();
}

void LastFMConfig::Validate() {
  ui_->busy->show();
  waiting_for_auth_ = true;

  service_->Authenticate(ui_->username->text(), ui_->password->text());
}

void LastFMConfig::AuthenticationComplete(bool success) {
  if (!waiting_for_auth_)
    return; // Wasn't us that was waiting for auth

  ui_->busy->hide();
  waiting_for_auth_ = false;

  if (success) {
    //clear password just to be sure
    ui_->password->clear();
    RefreshControls(true);
  } else {
    QMessageBox::warning(this, tr("Authentication failed"), tr("Your Last.fm credentials were incorrect"));
  }

  emit ValidationComplete(success);
}

void LastFMConfig::Load() {
  ui_->scrobble->setChecked(service_->IsScrobblingEnabled());
  ui_->love_ban_->setChecked(service_->AreButtonsVisible());
  ui_->scrobble_button->setChecked(service_->IsScrobbleButtonVisible());

  RefreshControls(service_->IsAuthenticated());
}

void LastFMConfig::Save() {
  QSettings s;
  s.beginGroup(LastFMService::kSettingsGroup);
  s.setValue("ScrobblingEnabled", ui_->scrobble->isChecked());
  s.setValue("ShowLoveBanButtons", ui_->love_ban_->isChecked());
  s.setValue("ShowScrobbleButton", ui_->scrobble_button->isChecked());
  s.endGroup();

  service_->ReloadSettings();
}

void LastFMConfig::SignOut() {
  ui_->username->clear();
  ui_->password->clear();
  RefreshControls(false);

  service_->SignOut();
}

void LastFMConfig::RefreshControls(bool authenticated) {
  ui_->groupBox->setVisible(!authenticated);
  ui_->sign_out->setVisible(authenticated);
  if (authenticated) {
    const bool is_subscriber = service_->IsSubscriber();
    const char* icon_path = is_subscriber ? kSubscribedIcon : kNotSubscribedIcon;
    ui_->icon->setPixmap(QIcon(icon_path).pixmap(16));
    ui_->status->setText(QString(tr("You're logged in as <b>%1</b>")).arg(lastfm::ws::Username));

    if (is_subscriber) {
      ui_->subscriber_warning->hide();
    } else {
      ui_->subscriber_warning->setText(
        tr("You will not be able to play Last.fm radio stations "
           "as you are not a Last.fm subscriber."));
    }
  }
  else {
    ui_->icon->setPixmap(IconLoader::Load("dialog-question").pixmap(16));
    ui_->status->setText(tr("Please fill in the blanks to login into Last.fm"));
    ui_->subscriber_warning->setText(
        tr("You can scrobble tracks for free, but only "
           "<span style=\" font-weight:600;\">paid subscribers</span> "
           "can stream Last.fm radio from Clementine."));
    ui_->subscriber_warning->show();
  }
}
