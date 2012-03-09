/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "gpoddersync.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/network.h"
#include "core/utilities.h"

#include <ApiRequest.h>

#include <QCoreApplication>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QSettings>

const char* GPodderSync::kSettingsGroup = "Podcasts";

GPodderSync::GPodderSync(Application* app, QObject* parent)
  : QObject(parent),
    app_(app),
    network_(new NetworkAccessManager(this))
{
  ReloadSettings();
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
}

GPodderSync::~GPodderSync() {
}

QString GPodderSync::DeviceId() {
  return QString("%1-%2").arg(qApp->applicationName(),
                              QHostInfo::localHostName()).toLower();
}

QString GPodderSync::DefaultDeviceName() {
  return tr("%1 on %2").arg(qApp->applicationName(),
                            QHostInfo::localHostName());
}

bool GPodderSync::is_logged_in() const {
  return !username_.isEmpty() && !password_.isEmpty() && api_;
}

void GPodderSync::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  username_ = s.value("gpodder_username").toString();
  password_ = s.value("gpodder_password").toString();

  if (!username_.isEmpty() && !password_.isEmpty()) {
    api_.reset(new mygpo::ApiRequest(username_, password_, network_));
  }
}

QNetworkReply* GPodderSync::Login(const QString& username, const QString& password,
                                  const QString& device_name) {
  api_.reset(new mygpo::ApiRequest(username, password, network_));

  QNetworkReply* reply = api_->renameDevice(
        username, DeviceId(), device_name,
        Utilities::IsLaptop() ? mygpo::Device::LAPTOP
                              : mygpo::Device::DESKTOP);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(LoginFinished(QNetworkReply*,QString,QString)),
             reply, username, password);
  return reply;
}

void GPodderSync::LoginFinished(QNetworkReply* reply,
                                const QString& username, const QString& password) {
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    username_ = username;
    password_ = password;

    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("gpodder_username", username);
    s.setValue("gpodder_password", password);
  } else {
    api_.reset();
  }
}

void GPodderSync::Logout() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("gpodder_username");
  s.remove("gpodder_password");

  api_.reset();
}
