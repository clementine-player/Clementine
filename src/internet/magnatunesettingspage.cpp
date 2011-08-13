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

#include "magnatunesettingspage.h"

#include "core/network.h"
#include "magnatuneservice.h"
#include "internetmodel.h"
#include "ui_magnatunesettingspage.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>

MagnatuneSettingsPage::MagnatuneSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    credentials_changed_(false),
    network_(new NetworkAccessManager(this)),
    ui_(new Ui_MagnatuneSettingsPage)
{
  ui_->setupUi(this);
  ui_->busy->hide();
  setWindowIcon(QIcon(":/providers/magnatune.png"));

  connect(ui_->membership, SIGNAL(currentIndexChanged(int)), SLOT(MembershipChanged(int)));

  connect(network_, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
          SLOT(AuthenticationRequired(QNetworkReply*, QAuthenticator*)));
  connect(ui_->username, SIGNAL(textEdited(const QString&)),
          SLOT(CredentialsChanged()));
  connect(ui_->password, SIGNAL(textEdited(const QString&)),
          SLOT(CredentialsChanged()));
  connect(ui_->login, SIGNAL(clicked()), SLOT(Login()));
}

MagnatuneSettingsPage::~MagnatuneSettingsPage() {
  delete ui_;
}

const char* kMagnatuneDownloadValidateUrl = "http://download.magnatune.com/";
const char* kMagnatuneStreamingValidateUrl = "http://streaming.magnatune.com/";

void MagnatuneSettingsPage::Login() {
  if (!credentials_changed_)
    return;
  ui_->busy->show();

  MagnatuneService::MembershipType type =
      MagnatuneService::MembershipType(ui_->membership->currentIndex());

  QUrl url(type == MagnatuneService::Membership_Streaming ?
           kMagnatuneStreamingValidateUrl :
           kMagnatuneDownloadValidateUrl,
           QUrl::StrictMode);

  url.setUserName(ui_->username->text());
  // NOTE: Magnatune actually only checks the first 8 characters.
  url.setPassword(ui_->password->text());

  QNetworkRequest req(url);
  // Disable keep-alives and gzip compression as it's broken on magnatune.
  req.setRawHeader("Connection", "Close");
  req.setRawHeader("Accept-Encoding", "identity");
  // Disable caching.
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network_->head(req);
  connect(reply, SIGNAL(finished()), SLOT(LoginFinished()));
}

void MagnatuneSettingsPage::LoginFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();

  ui_->busy->hide();

  const bool success =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
  credentials_changed_ = !success;
  if (!success) {
    QMessageBox::warning(
        this, tr("Authentication failed"), tr("Your Magnatune credentials were incorrect"));
  } else {
    Save();
  }
  ui_->login->setEnabled(!success);
}

void MagnatuneSettingsPage::AuthenticationRequired(
    QNetworkReply* reply, QAuthenticator*) {
  // We send the authentication with the first request so this means we got
  // a 401 Authentication Required, ie. the credentials are incorrect.
  reply->abort();
}

void MagnatuneSettingsPage::Load() {
  QSettings s;
  s.beginGroup(MagnatuneService::kSettingsGroup);

  ui_->membership->setCurrentIndex(s.value("membership", MagnatuneService::Membership_None).toInt());
  ui_->username->setText(s.value("username").toString());
  ui_->password->setText(s.value("password").toString());
  ui_->format->setCurrentIndex(s.value("format", MagnatuneService::Format_Ogg).toInt());

  credentials_changed_ = false;
}

void MagnatuneSettingsPage::Save() {
  QSettings s;
  s.beginGroup(MagnatuneService::kSettingsGroup);

  s.setValue("membership", ui_->membership->currentIndex());
  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());
  s.setValue("format", ui_->format->currentIndex());

  InternetModel::Service<MagnatuneService>()->ReloadSettings();
}

void MagnatuneSettingsPage::MembershipChanged(int value) {
  bool enabled = MagnatuneService::MembershipType(value) !=
                 MagnatuneService::Membership_None;
  ui_->login_container->setEnabled(enabled);
  ui_->preferences_group->setEnabled(enabled);
}

void MagnatuneSettingsPage::CredentialsChanged() {
  credentials_changed_ = true;
}
