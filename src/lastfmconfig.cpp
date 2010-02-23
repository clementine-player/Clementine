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
}

void LastFMConfig::Save() {
  QSettings s;
  s.beginGroup(LastFMService::kSettingsGroup);
  s.setValue("ScrobblingEnabled", ui_.scrobble->isChecked());
  s.endGroup();

  service_->ReloadSettings();
}
