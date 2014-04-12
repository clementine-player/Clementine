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

#include "ubuntuunityhack.h"
#include "core/logging.h"

#include <QFile>
#include <QProcess>

const char* UbuntuUnityHack::kGSettingsFileName = "gsettings";
const char* UbuntuUnityHack::kUnityPanel = "com.canonical.Unity.Panel";
const char* UbuntuUnityHack::kUnitySystrayWhitelist = "systray-whitelist";

UbuntuUnityHack::UbuntuUnityHack(QObject* parent) : QObject(parent) {
  // Check if we're on Ubuntu first.
  QFile lsb_release("/etc/lsb-release");
  if (lsb_release.open(QIODevice::ReadOnly)) {
    QByteArray data = lsb_release.readAll();
    if (!data.contains("DISTRIB_ID=Ubuntu")) {
      // It's not Ubuntu - don't do anything.
      return;
    }
  }

  // Get the systray whitelist from gsettings.  If this fails we're probably
  // not running on a system with unity
  QProcess* get = new QProcess(this);
  connect(get, SIGNAL(finished(int)), SLOT(GetFinished(int)));
  connect(get, SIGNAL(error(QProcess::ProcessError)), SLOT(GetError()));
  get->start(kGSettingsFileName, QStringList() << "get" << kUnityPanel
                                               << kUnitySystrayWhitelist);
}

void UbuntuUnityHack::GetError() {
  QProcess* get = qobject_cast<QProcess*>(sender());
  if (!get) {
    return;
  }

  get->deleteLater();
}

void UbuntuUnityHack::GetFinished(int exit_code) {
  QProcess* get = qobject_cast<QProcess*>(sender());
  if (!get) {
    return;
  }

  get->deleteLater();

  if (exit_code != 0) {
    // Probably not running in Unity.
    return;
  }

  QByteArray whitelist = get->readAllStandardOutput();

  qLog(Debug) << "Unity whitelist is" << whitelist;

  int index = whitelist.lastIndexOf(']');
  if (index == -1 || whitelist.contains("'clementine'")) {
    return;
  }

  whitelist = whitelist.left(index) + QString(", 'clementine'").toUtf8() +
              whitelist.mid(index);

  qLog(Debug) << "Setting unity whitelist to" << whitelist;

  QProcess* set = new QProcess(this);
  connect(set, SIGNAL(finished(int)), set, SLOT(deleteLater()));
  set->start(kGSettingsFileName, QStringList() << "set" << kUnityPanel
                                               << kUnitySystrayWhitelist
                                               << whitelist);

  qLog(Info) << "Clementine has added itself to the Unity system tray"
             << "whitelist, but this won't take effect until the next time"
             << "you log out and log back in.";
}
