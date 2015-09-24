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

#include "iconloader.h"
#include "networkproxysettingspage.h"
#include "ui_networkproxysettingspage.h"
#include "core/networkproxyfactory.h"

#include <QSettings>

NetworkProxySettingsPage::NetworkProxySettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_NetworkProxySettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("applications-internet", IconLoader::base));
}

NetworkProxySettingsPage::~NetworkProxySettingsPage() { delete ui_; }

void NetworkProxySettingsPage::Load() {
  QSettings s;

  s.beginGroup(NetworkProxyFactory::kSettingsGroup);
  NetworkProxyFactory::Mode mode = NetworkProxyFactory::Mode(
      s.value("mode", NetworkProxyFactory::Mode_System).toInt());
  switch (mode) {
    case NetworkProxyFactory::Mode_Manual:
      ui_->proxy_manual->setChecked(true);
      break;

    case NetworkProxyFactory::Mode_Direct:
      ui_->proxy_direct->setChecked(true);
      break;

    case NetworkProxyFactory::Mode_System:
    default:
      ui_->proxy_system->setChecked(true);
      break;
  }

  ui_->proxy_type->setCurrentIndex(
      s.value("type", QNetworkProxy::HttpProxy).toInt() ==
              QNetworkProxy::HttpProxy
          ? 0
          : 1);
  ui_->proxy_hostname->setText(s.value("hostname").toString());
  ui_->proxy_port->setValue(s.value("port").toInt());
  ui_->proxy_auth->setChecked(s.value("use_authentication", false).toBool());
  ui_->proxy_username->setText(s.value("username").toString());
  ui_->proxy_password->setText(s.value("password").toString());
  s.endGroup();
}

void NetworkProxySettingsPage::Save() {
  QSettings s;

  NetworkProxyFactory::Mode mode = NetworkProxyFactory::Mode_System;
  if (ui_->proxy_direct->isChecked())
    mode = NetworkProxyFactory::Mode_Direct;
  else if (ui_->proxy_system->isChecked())
    mode = NetworkProxyFactory::Mode_System;
  else if (ui_->proxy_manual->isChecked())
    mode = NetworkProxyFactory::Mode_Manual;

  s.beginGroup(NetworkProxyFactory::kSettingsGroup);
  s.setValue("mode", mode);
  s.setValue("type", ui_->proxy_type->currentIndex() == 0
                         ? QNetworkProxy::HttpProxy
                         : QNetworkProxy::Socks5Proxy);
  s.setValue("hostname", ui_->proxy_hostname->text());
  s.setValue("port", ui_->proxy_port->value());
  s.setValue("use_authentication", ui_->proxy_auth->isChecked());
  s.setValue("username", ui_->proxy_username->text());
  s.setValue("password", ui_->proxy_password->text());
  s.endGroup();

  NetworkProxyFactory::Instance()->ReloadSettings();
}
