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
#include "networkremotesettingspage.h"
#include "ui_networkremotesettingspage.h"
#include "networkremote/networkremote.h"
#include "networkremote/networkremotehelper.h"

#include <QSettings>


NetworkRemoteSettingsPage::NetworkRemoteSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_NetworkRemoteSettingsPage)
{
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("ipodtouchicon"));
}

NetworkRemoteSettingsPage::~NetworkRemoteSettingsPage() {
  delete ui_;
}

void NetworkRemoteSettingsPage::Load() {
  QSettings s;
  int port;

  s.beginGroup(NetworkRemote::kSettingsGroup);

  port = s.value("port").toInt();
  if (port == 0) {
      ui_->remote_port->setValue(NetworkRemote::kDefaultServerPort);
  }
  else {
      ui_->remote_port->setValue(s.value("port").toInt());
  }

  ui_->use_remote->setChecked(s.value("use_remote").toBool());
  if (s.contains("only_non_public_ip")) {
    ui_->only_non_public_ip->setChecked(s.value("only_non_public_ip").toBool());
  } else {
    // Default yes
    ui_->only_non_public_ip->setChecked(true);
    s.setValue("only_non_public_ip", true);
  }

  s.endGroup();
}

void NetworkRemoteSettingsPage::Save() {
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  s.setValue("port", ui_->remote_port->value());
  s.setValue("use_remote", ui_->use_remote->isChecked());
  s.setValue("only_non_public_ip", ui_->only_non_public_ip->isChecked());

  s.endGroup();

  if (NetworkRemoteHelper::Instance()) {
    NetworkRemoteHelper::Instance()->ReloadSettings();
  }
}
