/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2015 - 2016, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

#include "config.h"
#include "idlehandler.h"
#include "core/logging.h"

#include <QtGlobal>

#ifdef HAVE_DBUS
#include "dbusidlehandler.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#ifdef Q_OS_DARWIN
#include "macidlehandler.h"
#endif

#ifdef Q_OS_WIN32
#include "windowsidlehandler.h"
#endif

// Services
const char* IdleHandler::kGnomeScreensaverService
    = "org.gnome.ScreenSaver";
const char* IdleHandler::kFreedesktopScreensaverService
    = "org.freedesktop.ScreenSaver";
const char* IdleHandler::kGnomePowermanagerService
    = "org.gnome.SessionManager";
const char* IdleHandler::kFreedesktopPowermanagerService
    = "org.freedesktop.PowerManagement";

#ifdef HAVE_DBUS
namespace {
  // Gnome Screensaver
  const char* kGnomeScreensaverPath
      = "/";
  const char* kGnomeScreensaverInterface
      = "org.gnome.ScreenSaver";
  
  // Freedesktop screensaver
  const char* kFreedesktopScreensaverPath
      = "/ScreenSaver";
  const char* kFreedesktopScreensaverInterface
      = "org.freedesktop.ScreenSaver";

  // Gnome powermanager
  const char* kGnomePowermanagerPath
      = "/org/gnome/SessionManager";
  const char* kGnomePowermanagerInterface
      = "org.gnome.SessionManager";

  // Freedesktop powermanager
  const char* kFreedesktopPowermanagerPath
      = "/org/freedesktop/PowerManagement/Inhibit";
  const char* kFreedesktopPowermanagerInterface
      = "org.freedesktop.PowerManagement.Inhibit";
}
#endif

IdleHandler* IdleHandler::screensaver_ = 0;
IdleHandler* IdleHandler::suspend_ = 0;

IdleHandler::Inhibitor IdleHandler::inbtr_;

IdleHandler* IdleHandler::GetScreensaver() {
  inbtr_ = Screensaver;
  if (!screensaver_) {
#if defined(HAVE_DBUS)
    // For gnome
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
            kGnomeScreensaverService)) {
      screensaver_ = new DBusIdleHandler(kGnomeScreensaverService,
                                         kGnomeScreensaverPath,
                                         kGnomeScreensaverInterface);
    } else if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
                   kFreedesktopScreensaverService)) /* For KDE, XFCE & others */ {
      screensaver_ = new DBusIdleHandler(kFreedesktopScreensaverService,
                                         kFreedesktopScreensaverPath,
                                         kFreedesktopScreensaverInterface);
    } else {
      qLog(Warning) << "no supported dbus screensaver service available";
    }

#elif defined(Q_OS_DARWIN)
    screensaver_ = new MacIdleHandler();
#elif defined(Q_OS_WIN32)
    screensaver_ = new WindowsIdleHandler();
#endif
  }
  return screensaver_;
}

IdleHandler* IdleHandler::GetSuspend() {
  inbtr_ = Suspend;
  if (!suspend_) {
#if defined(HAVE_DBUS)
    // For gnome
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
            kGnomePowermanagerService)) {
      suspend_ = new DBusIdleHandler(kGnomePowermanagerService,
                                     kGnomePowermanagerPath,
                                     kGnomePowermanagerInterface);
    } else if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
                   kFreedesktopPowermanagerService)) /* For KDE, XFCE & others */ {
      suspend_ = new DBusIdleHandler(kFreedesktopPowermanagerService,
                                     kFreedesktopPowermanagerPath,
                                     kFreedesktopPowermanagerInterface);
    } else {
      qLog(Warning) << "no supported dbus powermanager service available";
    }

#elif defined(Q_OS_DARWIN)
    suspend_ = new MacIdleHandler();
#elif defined(Q_OS_WIN32)
    suspend_ = new WindowsIdleHandler();
#endif
  }
  return suspend_;
}
