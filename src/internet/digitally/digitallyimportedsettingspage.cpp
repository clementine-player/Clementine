/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>
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

#include "digitallyimportedsettingspage.h"
#include "ui_digitallyimportedsettingspage.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QSettings>

#include "digitallyimportedclient.h"
#include "digitallyimportedservicebase.h"
#include "core/closure.h"
#include "ui/iconloader.h"

DigitallyImportedSettingsPage::DigitallyImportedSettingsPage(
    SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_DigitallyImportedSettingsPage),
      client_(new DigitallyImportedClient("di", this)) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("digitallyimported", IconLoader::Provider));

  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));
  connect(ui_->login_state, SIGNAL(LoginClicked()), SLOT(Login()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));

  ui_->login_state->AddCredentialField(ui_->username);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->credential_group);

  ui_->login_state->SetAccountTypeText(tr("A premium account is required"));
  ui_->login_state->SetAccountTypeVisible(true);
}

DigitallyImportedSettingsPage::~DigitallyImportedSettingsPage() { delete ui_; }

void DigitallyImportedSettingsPage::Login() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoginInProgress);

  QNetworkReply* reply =
      client_->Auth(ui_->username->text(), ui_->password->text());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LoginFinished(QNetworkReply*)), reply);
}

void DigitallyImportedSettingsPage::LoginFinished(QNetworkReply* reply) {
  reply->deleteLater();

  DigitallyImportedClient::AuthReply result = client_->ParseAuthReply(reply);

  QString name = QString("%1 %2").arg(result.first_name_, result.last_name_);
  UpdateLoginState(result.listen_hash_, name, result.expires_);

  if (result.success_) {
    // Clear the password field just to be sure
    ui_->password->clear();

    // Save the listen key and account information
    QSettings s;
    s.beginGroup(DigitallyImportedServiceBase::kSettingsGroup);
    s.setValue("listen_hash", result.listen_hash_);
    s.setValue("full_name", name);
    s.setValue("expires", result.expires_);
  } else {
    QMessageBox::warning(this, tr("Authentication failed"),
                         result.error_reason_);
  }
}

void DigitallyImportedSettingsPage::UpdateLoginState(const QString& listen_hash,
                                                     const QString& name,
                                                     const QDateTime& expires) {
  if (listen_hash.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
    ui_->login_state->SetExpires(QDate());
    ui_->login_state->SetAccountTypeVisible(true);
  } else {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, name);
    ui_->login_state->SetExpires(expires.date());
    ui_->login_state->SetAccountTypeVisible(false);
  }

  ui_->premium_audio_type->setEnabled(!listen_hash.isEmpty());
  ui_->premium_audio_label->setEnabled(!listen_hash.isEmpty());
}

void DigitallyImportedSettingsPage::Logout() {
  QSettings s;
  s.beginGroup(DigitallyImportedServiceBase::kSettingsGroup);
  s.setValue("listen_hash", QString());
  s.setValue("full_name", QString());
  s.setValue("expires", QDateTime());

  UpdateLoginState(QString(), QString(), QDateTime());
}

void DigitallyImportedSettingsPage::Load() {
  QSettings s;
  s.beginGroup(DigitallyImportedServiceBase::kSettingsGroup);

  ui_->basic_audio_type->setCurrentIndex(
      s.value("basic_audio_type", 1).toInt());
  ui_->premium_audio_type->setCurrentIndex(
      s.value("premium_audio_type", 2).toInt());
  ui_->username->setText(s.value("username").toString());

  UpdateLoginState(s.value("listen_hash").toString(),
                   s.value("full_name").toString(),
                   s.value("expires").toDateTime());
}

void DigitallyImportedSettingsPage::Save() {
  QSettings s;
  s.beginGroup(DigitallyImportedServiceBase::kSettingsGroup);

  s.setValue("basic_audio_type", ui_->basic_audio_type->currentIndex());
  s.setValue("premium_audio_type", ui_->premium_audio_type->currentIndex());
  s.setValue("username", ui_->username->text());
}
