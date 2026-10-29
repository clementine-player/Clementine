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

#ifndef CORE_DBUSSLEEPINHIBITOR_H_
#define CORE_DBUSSLEEPINHIBITOR_H_

#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QMap>
#include <QObject>
#include <QString>

#include "core/sleepinhibitor.h"

class QDBusError;

// Asks the desktop through the XDG desktop portal, which works inside the
// Flatpak too. Without the portal's Inhibit interface, the display is kept on
// through org.freedesktop.ScreenSaver, and the computer awake through
// systemd-logind's idle inhibitor.
class DBusSleepInhibitor : public QObject, public SleepInhibitor::Backend {
  Q_OBJECT

 public:
  DBusSleepInhibitor();
  ~DBusSleepInhibitor();

  void Inhibit(SleepInhibitor::Keep keep, const QString& reason) override;
  void Uninhibit(SleepInhibitor::Keep keep) override;

 private:
  struct Inhibition {
    enum class State { Released, Requesting, Held };
    enum class Via { Portal, ScreenSaver, Logind };

    State state = State::Released;
    // Whether it's still needed: a request can be answered after it isn't.
    bool needed = false;
    QString reason;
    Via via = Via::Portal;
    QDBusObjectPath portal_request;
    quint32 screensaver_cookie = 0;
    // Held until it's closed.
    QDBusUnixFileDescriptor logind_fd;
  };

  void RequestFromPortal(SleepInhibitor::Keep keep);
  void RequestFromScreenSaver(SleepInhibitor::Keep keep);
  void RequestFromLogind(SleepInhibitor::Keep keep);
  void Granted(SleepInhibitor::Keep keep);
  void Refused(SleepInhibitor::Keep keep, const QDBusError& error);
  void Release(SleepInhibitor::Keep keep);

  QMap<SleepInhibitor::Keep, Inhibition> inhibitions_;
};

#endif  // CORE_DBUSSLEEPINHIBITOR_H_
