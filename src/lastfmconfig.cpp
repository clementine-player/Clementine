#include "lastfmconfig.h"
#include "lastfmservice.h"

#include <lastfm/ws.h>

#include <QMessageBox>

LastFMConfig::LastFMConfig(LastFMService* service, QWidget *parent)
  : QDialog(parent),
    service_(service)
{
  ui_.setupUi(this);
  ui_.busy->hide();

  connect(service_, SIGNAL(AuthenticationComplete(bool)), SLOT(AuthenticationComplete(bool)));
}

void LastFMConfig::accept() {
  if (ui_.username->text().isEmpty() || ui_.password->text().isEmpty()) {
    QDialog::accept();
    return;
  }

  ui_.busy->show();
  ui_.button_box->setEnabled(false);

  service_->Authenticate(ui_.username->text(), ui_.password->text());

  emit ScrobblingEnabledChanged(ui_.scrobble->isChecked());
}

void LastFMConfig::AuthenticationComplete(bool success) {
  ui_.busy->hide();
  ui_.button_box->setEnabled(true);

  if (success) {
    ui_.username->setText(lastfm::ws::Username);
    ui_.password->clear();
    QDialog::accept();
  } else {
    QMessageBox::warning(this, "Authentication failed", "Your Last.fm credentials were incorrect");
  }
}
