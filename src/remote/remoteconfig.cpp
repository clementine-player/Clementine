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

#include "remoteconfig.h"
#include "ui_remoteconfig.h"
#include "ui/iconloader.h"

#include "keychain.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QSettings>

const char* kClientLoginUrl = "https://www.google.com/accounts/ClientLogin";
const char* RemoteConfig::kSettingsGroup = "Remote";

RemoteConfig::RemoteConfig(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_RemoteConfig),
    waiting_for_auth_(false),
    network_(new NetworkAccessManager(this))
{
  ui_->setupUi(this);
  ui_->busy->hide();

  // Icons
  ui_->sign_out->setIcon(IconLoader::Load("list-remove"));

  connect(ui_->sign_out, SIGNAL(clicked()), SLOT(SignOut()));

  ui_->username->setMinimumWidth(QFontMetrics(QFont()).width("WWWWWWWWWWWW"));
  resize(sizeHint());
}

RemoteConfig::~RemoteConfig() {
  delete ui_;
}

bool RemoteConfig::NeedsValidation() const {
  return !ui_->username->text().isEmpty() && !ui_->password->text().isEmpty();
}

void RemoteConfig::Validate() {
  ui_->busy->show();
  waiting_for_auth_ = true;

  ValidateGoogleAccount(ui_->username->text(), ui_->password->text());
}

// Validates a Google account against ClientLogin:
// http://code.google.com/apis/accounts/docs/AuthForInstalledApps.html#ClientLogin
void RemoteConfig::ValidateGoogleAccount(const QString& username, const QString& password) {
  QNetworkRequest request = QNetworkRequest(QUrl(kClientLoginUrl));
  QString post_data =
      "accountType=HOSTED_OR_GOOGLE&"
      "service=mail&"
      "source=" + QUrl::toPercentEncoding(QCoreApplication::applicationName()) + "&"
      "Email=" + QUrl::toPercentEncoding(username) + "&"
      "Passwd=" + QUrl::toPercentEncoding(password);
  QNetworkReply* reply = network_->post(request, post_data.toUtf8());
  connect(reply, SIGNAL(finished()), SLOT(ValidateFinished()));
}

void RemoteConfig::ValidateFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();

  QVariant status_code =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (reply->error() != QNetworkReply::NoError || !status_code.isValid() || status_code.toInt() != 200) {
    AuthenticationComplete(false);
    return;
  }
  AuthenticationComplete(true);
}

void RemoteConfig::AuthenticationComplete(bool success) {
  if (!waiting_for_auth_)
    return; // Wasn't us that was waiting for auth

  ui_->busy->hide();
  waiting_for_auth_ = false;

  if (success) {
    validated_password_ = ui_->password->text();
    ui_->password->clear();
  } else {
    QMessageBox::warning(this, tr("Authentication failed"), tr("Your Google credentials were incorrect"));
  }

  emit ValidationComplete(success);
}

void RemoteConfig::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QVariant username = s.value("username");
  if (username.isValid()) {
    ui_->username->setText(username.toString());
  }
}

void RemoteConfig::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  const QString& username = ui_->username->text();
  s.setValue("username", username);
  Keychain* keychain = Keychain::getDefault();
  keychain->setPassword(username, validated_password_);
  validated_password_.clear();
}

void RemoteConfig::SignOut() {
  ui_->username->clear();
  ui_->password->clear();
  ui_->sign_out->setEnabled(false);
}
