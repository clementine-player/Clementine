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

#include "screensaver.h"

#include <QtGlobal>

#include "config.h"

#ifdef HAVE_DBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "dbusscreensaver.h"
#endif

#ifdef Q_OS_DARWIN
#include "macscreensaver.h"
#endif

#ifdef Q_OS_WIN32
#include "windowsscreensaver.h"
#endif

#include <QtDebug>

const char* Screensaver::kGnomeService = "org.gnome.ScreenSaver";
const char* Screensaver::kGnomePath = "/";
const char* Screensaver::kGnomeInterface = "org.gnome.ScreenSaver";
const char* Screensaver::kKdeService = "org.kde.ScreenSaver";
const char* Screensaver::kKdePath = "/ScreenSaver/";
const char* Screensaver::kKdeInterface = "org.freedesktop.ScreenSaver";

Screensaver* Screensaver::screensaver_ = 0;

Screensaver* Screensaver::GetScreensaver() {
  if (!screensaver_) {
#if defined(HAVE_DBUS)
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
            kGnomeService)) {
      screensaver_ =
          new DBusScreensaver(kGnomeService, kGnomePath, kGnomeInterface);
    } else if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
                   kKdeService)) {
      screensaver_ = new DBusScreensaver(kKdeService, kKdePath, kKdeInterface);
    }
#elif defined(Q_OS_DARWIN)
    screensaver_ = new MacScreensaver();
#elif defined(Q_OS_WIN32)
    screensaver_ = new WindowsScreensaver();
#endif
  }
  return screensaver_;
}
