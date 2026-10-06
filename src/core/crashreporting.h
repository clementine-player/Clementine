/* This file is part of Clementine.
   Copyright 2011, David Sansome <davidsansome@gmail.com>
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

#ifndef CORE_CRASHREPORTING_H_
#define CORE_CRASHREPORTING_H_

#include <QString>

class QWidget;

// Sends crash reports to Sentry, but only one at a time and only when the user
// says so.
//
// While an instance of this class is alive, crashes are caught by
// sentry-native's Crashpad handler, which runs in a process of its own and
// saves a minidump. Sentry is started with user consent required and consent
// is never left given, so the handler doesn't upload anything itself. On the
// next start the minidumps are set aside, and AskToSendPendingReports() asks
// the user whether to send them.
//
// Builds without ENABLE_SENTRY, or with an empty SENTRY_DSN, get no-op stubs.
class CrashReporting {
 public:
  static const char* kSettingsGroup;
  static const char* kAskAfterCrash;

  CrashReporting();
  ~CrashReporting();

  // True if this build can send crash reports at all.
  static bool IsAvailable();

  // If Clementine crashed since the user was last asked, asks whether to send
  // the reports, and sends or deletes them.
  static void AskToSendPendingReports(QWidget* parent);

  // Whether to catch crashes and offer to send a report afterwards. Changing
  // it takes effect the next time Clementine starts.
  static bool IsEnabled();
  static void SetEnabled(bool enabled);

 private:
  Q_DISABLE_COPY(CrashReporting)

  static QString DatabasePath();

  static bool sInitialised;
};

#endif  // CORE_CRASHREPORTING_H_
