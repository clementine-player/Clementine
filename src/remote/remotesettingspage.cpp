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

#include "remotesettingspage.h"
#include "ui_remotesettingspage.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QHostInfo>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSettings>

const char* kClientLoginUrl = "https://www.google.com/accounts/ClientLogin";
const char* RemoteSettingsPage::kSettingsGroup = "Remote";

RemoteSettingsPage::RemoteSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_RemoteSettingsPage),
    waiting_for_auth_(false),
    network_(new NetworkAccessManager(this))
{
  ui_->setupUi(this);
  ui_->busy->hide();
  ui_->icon->setPixmap(IconLoader::Load("task-reject").pixmap(16));
  setWindowIcon(IconLoader::Load("network-server"));

  // Icons
  ui_->sign_out->setIcon(IconLoader::Load("list-remove"));
  ui_->sign_out->hide();

  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));
  connect(ui_->sign_out, SIGNAL(clicked()), SLOT(SignOut()));

  ui_->username->setMinimumWidth(QFontMetrics(QFont()).width("WWWWWWWWWWWW"));
  resize(sizeHint());
}

QString RemoteSettingsPage::DefaultAgentName() {
  return QString("%1 on %2").arg(QCoreApplication::applicationName(),
                                 QHostInfo::localHostName());
}

RemoteSettingsPage::~RemoteSettingsPage() {
  delete ui_;
}

void RemoteSettingsPage::Login() {
  ui_->busy->show();
  waiting_for_auth_ = true;

  ValidateGoogleAccount(ui_->username->text(), ui_->password->text());
}

// Validates a Google account against ClientLogin and fetches a token usable for
// X-GOOGLE-TOKEN SASL authentication for Google Talk:
// http://code.google.com/apis/accounts/docs/AuthForInstalledApps.html#ClientLogin
void RemoteSettingsPage::ValidateGoogleAccount(const QString& username, const QString& password) {
  QNetworkRequest request = QNetworkRequest(QUrl(kClientLoginUrl));
  QString post_data =
      "accountType=HOSTED_OR_GOOGLE&"
      "service=mail&"
      "source=" + QUrl::toPercentEncoding(QCoreApplication::applicationName()) + "&"
      "Email=" + QUrl::toPercentEncoding(username) + "&"
      "Passwd=" + QUrl::toPercentEncoding(password);
  QNetworkReply* reply = network_->post(request, post_data.toUtf8());
  connect(reply, SIGNAL(finished()), SLOT(LoginFinished()));
}

void RemoteSettingsPage::LoginFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();
  QString data = QString::fromUtf8(reply->readAll());

  QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  if (reply->error() == QNetworkReply::NoError && status_code.isValid() && status_code.toInt() == 200) {
    QStringList params = data.split('\n');
    foreach (const QString& param, params) {
      if (param.startsWith("Auth=")) {
        LoginComplete(param.split('=')[1]);
        return;
      }
    }
  }
  LoginComplete(QString::null);
}

void RemoteSettingsPage::LoginComplete(const QString& token) {
  if (!waiting_for_auth_)
    return; // Wasn't us that was waiting for auth

  const bool success = !token.isNull();

  ui_->busy->hide();
  waiting_for_auth_ = false;

  if (!success) {
    QMessageBox::warning(this, tr("Authentication failed"), tr("Your Google credentials were incorrect"));
  } else {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("password", ui_->password->text());
    s.setValue("token", token);
    ui_->password->clear();

    //Save the other settings
    Save();

    ui_->login_details->hide();
    ui_->icon->setPixmap(IconLoader::Load("task-complete").pixmap(16));
    ui_->status->setText(QString(tr("You're logged in as <b>%1</b>")).arg(ui_->username->text()));
    ui_->sign_out->show();
  }
}

void RemoteSettingsPage::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  ui_->username->setText(s.value("username").toString());
  ui_->agent_name->setText(s.value("agent_name", DefaultAgentName()).toString());
}

void RemoteSettingsPage::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("username", ui_->username->text());
  s.setValue("agent_name", ui_->agent_name->text());
}

void RemoteSettingsPage::SignOut() {
  ui_->username->clear();
  ui_->password->clear();
  ui_->sign_out->hide();
  ui_->icon->setPixmap(IconLoader::Load("task-reject").pixmap(16));
  ui_->status->setText(QString(tr("Clementine can be controlled remotely by an Android phone.  To enable this feature log in with the same Google account that is configured on your phone.")));
  ui_->login_details->show();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("password", QString());
}
