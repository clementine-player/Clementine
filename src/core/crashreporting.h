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

#include <QtGlobal>

class QWidget;

// Sends crash reports to Sentry. While an instance of this class is alive,
// crashes are caught by sentry-native's Crashpad handler, which runs in a
// process of its own and uploads the minidump.
//
// Nothing is uploaded until the user has agreed to it: Sentry is started with
// user consent required, and consent is only given when the "send_reports"
// setting is true. Builds without ENABLE_SENTRY, or with an empty SENTRY_DSN,
// get no-op stubs.
class CrashReporting {
 public:
  static const char* kSettingsGroup;
  static const char* kSendReports;

  CrashReporting();
  ~CrashReporting();

  // True if this build can send crash reports at all.
  static bool IsAvailable();

  // Asks the user whether to send crash reports, unless they've already
  // answered. Their answer is remembered and can be changed in Preferences.
  static void AskForConsentIfNeeded(QWidget* parent);

  // Whether the user has agreed to send crash reports.
  static bool IsEnabled();
  static void SetEnabled(bool enabled);

 private:
  Q_DISABLE_COPY(CrashReporting)

  static bool sInitialised;
};

#endif  // CORE_CRASHREPORTING_H_
