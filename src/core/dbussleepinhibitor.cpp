/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "core/dbussleepinhibitor.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QVariantMap>

#include "core/logging.h"

namespace {

const char* kPortalService = "org.freedesktop.portal.Desktop";
const char* kPortalPath = "/org/freedesktop/portal/desktop";
const char* kPortalInterface = "org.freedesktop.portal.Inhibit";
const char* kPortalRequestInterface = "org.freedesktop.portal.Request";

// The portal's Inhibit flags. Idle keeps the session from going idle, so the
// screen neither blanks nor locks; Suspend only keeps the computer awake.
const quint32 kPortalSuspend = 4;
const quint32 kPortalIdle = 8;

const char* kScreenSaverService = "org.freedesktop.ScreenSaver";
const char* kScreenSaverPath = "/org/freedesktop/ScreenSaver";
const char* kScreenSaverInterface = "org.freedesktop.ScreenSaver";

const char* kLogindService = "org.freedesktop.login1";
const char* kLogindPath = "/org/freedesktop/login1";
const char* kLogindInterface = "org.freedesktop.login1.Manager";

}  // namespace

DBusSleepInhibitor::DBusSleepInhibitor() {}

// Requests still waiting for an answer are let go of by the desktop when
// Clementine's connection to the bus closes.
DBusSleepInhibitor::~DBusSleepInhibitor() {
  for (auto it = inhibitions_.begin(); it != inhibitions_.end(); ++it) {
    if (it->state == Inhibition::State::Held) Release(it.key());
  }
}

void DBusSleepInhibitor::Inhibit(SleepInhibitor::Keep keep,
                                 const QString& reason) {
  Inhibition* inhibition = &inhibitions_[keep];
  inhibition->needed = true;
  inhibition->reason = reason;
  if (inhibition->state == Inhibition::State::Released) {
    RequestFromPortal(keep);
  }
}

void DBusSleepInhibitor::Uninhibit(SleepInhibitor::Keep keep) {
  Inhibition* inhibition = &inhibitions_[keep];
  inhibition->needed = false;
  // A request still waiting for its answer is released when it comes.
  if (inhibition->state == Inhibition::State::Held) Release(keep);
}

void DBusSleepInhibitor::RequestFromPortal(SleepInhibitor::Keep keep) {
  Inhibition* inhibition = &inhibitions_[keep];
  inhibition->state = Inhibition::State::Requesting;

  QVariantMap options;
  options["reason"] = inhibition->reason;
  QDBusMessage message = QDBusMessage::createMethodCall(
      kPortalService, kPortalPath, kPortalInterface, "Inhibit");
  // No parent window: there might not be one on screen.
  message << QString()
          << (keep == SleepInhibitor::Keep::DisplayOn ? kPortalIdle
                                                      : kPortalSuspend)
          << options;

  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(
      QDBusConnection::sessionBus().asyncCall(message), this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this, keep](QDBusPendingCallWatcher* watcher) {
            watcher->deleteLater();
            QDBusPendingReply<QDBusObjectPath> reply = *watcher;
            if (reply.isError()) {
              qLog(Debug) << "The desktop portal couldn't inhibit sleep:"
                          << reply.error().message();
              if (keep == SleepInhibitor::Keep::DisplayOn) {
                RequestFromScreenSaver(keep);
              } else {
                RequestFromLogind(keep);
              }
              return;
            }
            Inhibition* inhibition = &inhibitions_[keep];
            inhibition->via = Inhibition::Via::Portal;
            inhibition->portal_request = reply.value();
            Granted(keep);
          });
}

void DBusSleepInhibitor::RequestFromScreenSaver(SleepInhibitor::Keep keep) {
  QDBusMessage message = QDBusMessage::createMethodCall(
      kScreenSaverService, kScreenSaverPath, kScreenSaverInterface, "Inhibit");
  message << QCoreApplication::applicationName() << inhibitions_[keep].reason;

  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(
      QDBusConnection::sessionBus().asyncCall(message), this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this, keep](QDBusPendingCallWatcher* watcher) {
            watcher->deleteLater();
            QDBusPendingReply<quint32> reply = *watcher;
            if (reply.isError()) {
              Refused(keep, reply.error());
              return;
            }
            Inhibition* inhibition = &inhibitions_[keep];
            inhibition->via = Inhibition::Via::ScreenSaver;
            inhibition->screensaver_cookie = reply.value();
            Granted(keep);
          });
}

void DBusSleepInhibitor::RequestFromLogind(SleepInhibitor::Keep keep) {
  // An idle inhibitor stops the computer suspending by itself, but not when
  // the user asks: "sleep" would stop that too.
  QDBusMessage message = QDBusMessage::createMethodCall(
      kLogindService, kLogindPath, kLogindInterface, "Inhibit");
  message << QString("idle") << QCoreApplication::applicationName()
          << inhibitions_[keep].reason << QString("block");

  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(
      QDBusConnection::systemBus().asyncCall(message), this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this, keep](QDBusPendingCallWatcher* watcher) {
            watcher->deleteLater();
            QDBusPendingReply<QDBusUnixFileDescriptor> reply = *watcher;
            if (reply.isError()) {
              Refused(keep, reply.error());
              return;
            }
            Inhibition* inhibition = &inhibitions_[keep];
            inhibition->via = Inhibition::Via::Logind;
            inhibition->logind_fd = reply.value();
            Granted(keep);
          });
}

void DBusSleepInhibitor::Granted(SleepInhibitor::Keep keep) {
  Inhibition* inhibition = &inhibitions_[keep];
  inhibition->state = Inhibition::State::Held;
  if (!inhibition->needed) {
    Release(keep);
  }
}

void DBusSleepInhibitor::Refused(SleepInhibitor::Keep keep,
                                 const QDBusError& error) {
  qLog(Warning) << "Couldn't keep the computer awake:" << error.message();
  // Tried again the next time it's needed.
  inhibitions_[keep].state = Inhibition::State::Released;
}

void DBusSleepInhibitor::Release(SleepInhibitor::Keep keep) {
  Inhibition* inhibition = &inhibitions_[keep];
  inhibition->state = Inhibition::State::Released;

  QDBusMessage message;
  switch (inhibition->via) {
    case Inhibition::Via::Portal:
      message = QDBusMessage::createMethodCall(
          kPortalService, inhibition->portal_request.path(),
          kPortalRequestInterface, "Close");
      break;
    case Inhibition::Via::ScreenSaver:
      message =
          QDBusMessage::createMethodCall(kScreenSaverService, kScreenSaverPath,
                                         kScreenSaverInterface, "UnInhibit");
      message << inhibition->screensaver_cookie;
      break;
    case Inhibition::Via::Logind:
      // Closing the descriptor lets go.
      inhibition->logind_fd = QDBusUnixFileDescriptor();
      return;
  }
  QDBusConnection::sessionBus().send(message);
}
