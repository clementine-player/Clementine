/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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
#include "core/closure.h"
#include "core/logging.h"
#include "globalshortcuts.h"
#include "gnomeglobalshortcutbackend.h"

#ifdef HAVE_DBUS
#include <dbus/gnomesettingsdaemon.h>
#endif

#include <QAction>
#include <QCoreApplication>
#include <QDateTime>
#include <QtDebug>

#ifdef HAVE_DBUS
#include <QtDBus>
#endif

const char* GnomeGlobalShortcutBackend::kGsdService =
    "org.gnome.SettingsDaemon.MediaKeys";
const char* GnomeGlobalShortcutBackend::kGsdService2 =
    "org.gnome.SettingsDaemon";
const char* GnomeGlobalShortcutBackend::kGsdPath =
    "/org/gnome/SettingsDaemon/MediaKeys";

GnomeGlobalShortcutBackend::GnomeGlobalShortcutBackend(GlobalShortcuts* parent)
    : GlobalShortcutBackend(parent),
      interface_(nullptr),
      is_connected_(false) {}

bool GnomeGlobalShortcutBackend::DoRegister() {
#ifdef HAVE_DBUS

  qLog(Debug) << "registering";

  if (!interface_) {
    // Check if the GSD service is available
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
            kGsdService)) {
      interface_ = new OrgGnomeSettingsDaemonMediaKeysInterface(
          kGsdService, kGsdPath, QDBusConnection::sessionBus(), this);
    } else if (QDBusConnection::sessionBus().interface()->isServiceRegistered(
                   kGsdService2)) {
      interface_ = new OrgGnomeSettingsDaemonMediaKeysInterface(
          kGsdService2, kGsdPath, QDBusConnection::sessionBus(), this);
    }
  }

  if (!interface_) {
    qLog(Warning) << "gnome settings daemon not registered";
    return false;
  }

  QDBusPendingReply<> reply =
      interface_->GrabMediaPlayerKeys(QCoreApplication::applicationName(),
                                      QDateTime::currentDateTime().toTime_t());

  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  NewClosure(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this,
             SLOT(RegisterFinished(QDBusPendingCallWatcher*)), watcher);

  return true;
#else  // HAVE_DBUS
  qLog(Warning) << "dbus not available";
  return false;
#endif
}

void GnomeGlobalShortcutBackend::RegisterFinished(
    QDBusPendingCallWatcher* watcher) {
#ifdef HAVE_DBUS
  QDBusMessage reply = watcher->reply();
  watcher->deleteLater();

  if (reply.type() == QDBusMessage::ErrorMessage) {
    qLog(Warning) << "Failed to grab media keys" << reply.errorName()
                  << reply.errorMessage();
    return;
  }

  connect(interface_, SIGNAL(MediaPlayerKeyPressed(QString, QString)), this,
          SLOT(GnomeMediaKeyPressed(QString, QString)));
  is_connected_ = true;

  qLog(Debug) << "registered";
#endif  // HAVE_DBUS
}

void GnomeGlobalShortcutBackend::DoUnregister() {
  qLog(Debug) << "unregister";
#ifdef HAVE_DBUS
  // Check if the GSD service is available
  if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(
           kGsdService))
    return;
  if (!interface_ || !is_connected_) return;

  is_connected_ = false;

  interface_->ReleaseMediaPlayerKeys(QCoreApplication::applicationName());
  disconnect(interface_, SIGNAL(MediaPlayerKeyPressed(QString, QString)), this,
             SLOT(GnomeMediaKeyPressed(QString, QString)));
#endif
}

void GnomeGlobalShortcutBackend::GnomeMediaKeyPressed(const QString&,
                                                      const QString& key) {
  if (key == "Play") manager_->shortcuts()["play_pause"].action->trigger();
  if (key == "Stop") manager_->shortcuts()["stop"].action->trigger();
  if (key == "Next") manager_->shortcuts()["next_track"].action->trigger();
  if (key == "Previous") manager_->shortcuts()["prev_track"].action->trigger();
}
