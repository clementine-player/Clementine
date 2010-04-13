/* This file is part of Clementine.

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

#include "globalshortcuts.h"
#include "qxtglobalshortcut.h"

#include "mac_startup.h"

#include <QtDebug>

#ifdef QT_DBUS_LIB
#  include <QtDBus>
#endif

const char* GlobalShortcuts::kGsdService = "org.gnome.SettingsDaemon";
const char* GlobalShortcuts::kGsdPath = "/org/gnome/SettingsDaemon/MediaKeys";
const char* GlobalShortcuts::kGsdInterface = "org.gnome.SettingsDaemon.MediaKeys";

GlobalShortcuts::GlobalShortcuts(QObject *parent)
  : QObject(parent)
{
  Init();
}

void GlobalShortcuts::Init() {
#ifdef Q_OS_DARWIN
  mac::SetShortcutHandler(this);
  return;
#endif
  if (RegisterGnome()) return;
  if (RegisterQxt()) return;
}

bool GlobalShortcuts::RegisterGnome() {
#ifdef QT_DBUS_LIB
  // Check if the GSD service is available
  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kGsdService))
    return false;

  QDBusInterface* interface = new QDBusInterface(
      kGsdService, kGsdPath, kGsdInterface, QDBusConnection::sessionBus(), this);

  connect(interface, SIGNAL(MediaPlayerKeyPressed(QString,QString)),
          this, SLOT(GnomeMediaKeyPressed(QString,QString)));

  return true;
#else // QT_DBUS_LIB
  return false;
#endif
}

bool GlobalShortcuts::RegisterQxt() {
#ifndef Q_OS_DARWIN
  QxtGlobalShortcut* play_pause = new QxtGlobalShortcut(QKeySequence("Media Play"), this);
  QxtGlobalShortcut* stop = new QxtGlobalShortcut(QKeySequence("Media Stop"), this);
  QxtGlobalShortcut* next = new QxtGlobalShortcut(QKeySequence("Media Next"), this);
  QxtGlobalShortcut* prev = new QxtGlobalShortcut(QKeySequence("Media Previous"), this);

  connect(play_pause, SIGNAL(activated()), SIGNAL(PlayPause()));
  connect(stop, SIGNAL(activated()), SIGNAL(Stop()));
  connect(next, SIGNAL(activated()), SIGNAL(Next()));
  connect(prev, SIGNAL(activated()), SIGNAL(Previous()));
#endif

  return true;
}

void GlobalShortcuts::GnomeMediaKeyPressed(const QString&, const QString& key) {
  if (key == "Play")     emit PlayPause();
  if (key == "Stop")     emit Stop();
  if (key == "Next")     emit Next();
  if (key == "Previous") emit Previous();
}

void GlobalShortcuts::MacMediaKeyPressed(const QString& key) {
  if (key == "Play")     emit PlayPause();
  // Stop doesn't exist on a mac keyboard.
  if (key == "Next")     emit Next();
  if (key == "Previous") emit Previous();
}
