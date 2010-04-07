/* This file is part of Clementine.

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

#include <lastfm/ws.h>

#include <QMessageBox>
#include <QSettings>

LastFMConfig::LastFMConfig(QWidget *parent)
  : QWidget(parent),
    service_(static_cast<LastFMService*>(RadioModel::ServiceByName("Last.fm")))
{
  ui_.setupUi(this);
  ui_.busy->hide();

  connect(service_, SIGNAL(AuthenticationComplete(bool)), SLOT(AuthenticationComplete(bool)));
  connect(ui_.sign_out, SIGNAL(clicked()), SLOT(SignOut()));
}

bool LastFMConfig::NeedsValidation() const {
  return !ui_.username->text().isEmpty() && !ui_.password->text().isEmpty();
}

void LastFMConfig::Validate() {
  ui_.busy->show();

  service_->Authenticate(ui_.username->text(), ui_.password->text());
}

void LastFMConfig::AuthenticationComplete(bool success) {
  if (!ui_.busy->isVisible())
    return; // Wasn't us that was waiting for auth

  ui_.busy->hide();

  if (success) {
    ui_.username->setText(lastfm::ws::Username);
    ui_.password->clear();
  } else {
    QMessageBox::warning(this, tr("Authentication failed"), tr("Your Last.fm credentials were incorrect"));
  }

  emit ValidationComplete(success);
}

void LastFMConfig::Load() {
  ui_.username->setText(lastfm::ws::Username);
  ui_.scrobble->setChecked(service_->IsScrobblingEnabled());
  ui_.love_ban_->setChecked(service_->AreButtonsVisible());
  ui_.sign_out->setEnabled(!lastfm::ws::SessionKey.isEmpty());
}

void LastFMConfig::Save() {
  QSettings s;
  s.beginGroup(LastFMService::kSettingsGroup);
  s.setValue("ScrobblingEnabled", ui_.scrobble->isChecked());
  s.setValue("ShowLoveBanButtons", ui_.love_ban_->isChecked());
  s.endGroup();

  service_->ReloadSettings();
}

void LastFMConfig::SignOut() {
  ui_.username->clear();
  ui_.password->clear();
  ui_.sign_out->setEnabled(false);
  service_->SignOut();
}
