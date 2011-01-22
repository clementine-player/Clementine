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

#include "gnomeglobalshortcutbackend.h"
#include "globalshortcuts.h"

#ifdef QT_DBUS_LIB
#  include "dbus/gnomesettingsdaemon.h"
#endif

#include <QAction>
#include <QtDebug>

#ifdef QT_DBUS_LIB
#  include <QtDBus>
#endif

const char* GnomeGlobalShortcutBackend::kGsdService = "org.gnome.SettingsDaemon";
const char* GnomeGlobalShortcutBackend::kGsdPath = "/org/gnome/SettingsDaemon/MediaKeys";
const char* GnomeGlobalShortcutBackend::kGsdInterface = "org.gnome.SettingsDaemon.MediaKeys";

GnomeGlobalShortcutBackend::GnomeGlobalShortcutBackend(GlobalShortcuts* parent)
  : GlobalShortcutBackend(parent),
    interface_(NULL)
{
}

bool GnomeGlobalShortcutBackend::DoRegister() {
#ifdef QT_DBUS_LIB
  qDebug() << __PRETTY_FUNCTION__ << "- starting";
  // Check if the GSD service is available
  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kGsdService)) {
    qDebug() << __PRETTY_FUNCTION__ << "- gnome settings daemon not registered";
    return false;
  }

  if (!interface_) {
    interface_ = new OrgGnomeSettingsDaemonMediaKeysInterface(
        kGsdService, kGsdPath, QDBusConnection::sessionBus(), this);
  }

  connect(interface_, SIGNAL(MediaPlayerKeyPressed(QString,QString)),
          this, SLOT(GnomeMediaKeyPressed(QString,QString)));

  qDebug() << __PRETTY_FUNCTION__ << "- complete";

  return true;
#else // QT_DBUS_LIB
  qDebug() << __PRETTY_FUNCTION__ << "- dbus not available";
  return false;
#endif
}

void GnomeGlobalShortcutBackend::DoUnregister() {
  qDebug() << __PRETTY_FUNCTION__;
#ifdef QT_DBUS_LIB
  // Check if the GSD service is available
  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(kGsdService))
    return;
  if (!interface_)
    return;

  disconnect(interface_, SIGNAL(MediaPlayerKeyPressed(QString,QString)),
             this, SLOT(GnomeMediaKeyPressed(QString,QString)));
#endif
}

void GnomeGlobalShortcutBackend::GnomeMediaKeyPressed(const QString&, const QString& key) {
  if (key == "Play")     manager_->shortcuts()["play_pause"].action->trigger();
  if (key == "Stop")     manager_->shortcuts()["stop"].action->trigger();
  if (key == "Next")     manager_->shortcuts()["next_track"].action->trigger();
  if (key == "Previous") manager_->shortcuts()["prev_track"].action->trigger();
}
