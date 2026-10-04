/* This file is part of Clementine.
   Copyright 2011, David Sansome <davidsansome@gmail.com>
   Copyright 2011, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "crashreporting.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include "config.h"
#include "core/logging.h"
#include "core/utilities.h"

#ifdef HAVE_SENTRY
#include <sentry.h>
#endif

const char* CrashReporting::kSettingsGroup = "CrashReporting";
const char* CrashReporting::kSendReports = "send_reports";
bool CrashReporting::sInitialised = false;

#ifdef HAVE_SENTRY

namespace {

#ifdef Q_OS_WIN32
void SetDatabasePath(sentry_options_t* options, const QString& path) {
  sentry_options_set_database_pathw(
      options, reinterpret_cast<const wchar_t*>(path.utf16()));
}

void SetHandlerPath(sentry_options_t* options, const QString& path) {
  sentry_options_set_handler_pathw(
      options, reinterpret_cast<const wchar_t*>(path.utf16()));
}
#else
void SetDatabasePath(sentry_options_t* options, const QString& path) {
  sentry_options_set_database_path(options,
                                   QFile::encodeName(path).constData());
}

void SetHandlerPath(sentry_options_t* options, const QString& path) {
  sentry_options_set_handler_path(options, QFile::encodeName(path).constData());
}
#endif

}  // namespace

CrashReporting::CrashReporting() {
  if (!IsAvailable()) return;

  sentry_options_t* options = sentry_options_new();
  sentry_options_set_dsn(options, CLEMENTINE_SENTRY_DSN);
  sentry_options_set_release(options,
                             QString("clementine@%1")
                                 .arg(QCoreApplication::applicationVersion())
                                 .toUtf8()
                                 .constData());

  // The handler doesn't upload anything until sentry_user_consent_give() is
  // called.
  sentry_options_set_require_user_consent(options, 1);

  // Only crashes, no usage data.
  sentry_options_set_auto_session_tracking(options, 0);

  // Reports and the Crashpad database live with the rest of Clementine's
  // files, so portable installs keep them in their own directory.
  SetDatabasePath(options, Utilities::GetConfigPath(Utilities::Path_Root) +
                               "/crashreports");

  // Packaged builds put crashpad_handler next to the executable, which is
  // where sentry-native looks by default. A build that's run from its build
  // directory doesn't, so fall back to the one it was built against.
  const QString bundled_handler =
      QDir(QCoreApplication::applicationDirPath())
          .filePath("crashpad_handler" CMAKE_EXECUTABLE_SUFFIX);
  if (!QFile::exists(bundled_handler) &&
      QFile::exists(CLEMENTINE_CRASHPAD_HANDLER)) {
    SetHandlerPath(options, CLEMENTINE_CRASHPAD_HANDLER);
  }

  if (sentry_init(options) != 0) {
    qLog(Warning) << "Failed to start crash reporting";
    return;
  }
  sInitialised = true;

  // Sentry remembers consent itself too, but the setting is what the user
  // sees in Preferences, so it wins.
  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.value(kSendReports, false).toBool()) {
    sentry_user_consent_give();
  } else {
    sentry_user_consent_revoke();
  }
}

CrashReporting::~CrashReporting() {
  if (sInitialised) {
    sentry_close();
    sInitialised = false;
  }
}

bool CrashReporting::IsAvailable() {
  return qstrlen(CLEMENTINE_SENTRY_DSN) != 0;
}

#else  // HAVE_SENTRY

CrashReporting::CrashReporting() {}

CrashReporting::~CrashReporting() {}

bool CrashReporting::IsAvailable() { return false; }

#endif  // HAVE_SENTRY

void CrashReporting::AskForConsentIfNeeded(QWidget* parent) {
  if (!IsAvailable()) return;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.contains(kSendReports)) return;

  QMessageBox box(QMessageBox::Question, QObject::tr("Send crash reports?"),
                  QObject::tr("If Clementine crashes, it can send a report to "
                              "its developers so they can fix the problem."),
                  QMessageBox::NoButton, parent);
  box.setInformativeText(
      QObject::tr("A report says what Clementine was doing when it crashed. "
                  "It can include file names and the titles of songs you "
                  "were playing. You can change your mind at any time in "
                  "Preferences."));
  QPushButton* send =
      box.addButton(QObject::tr("Send reports"), QMessageBox::AcceptRole);
  box.addButton(QObject::tr("Don't send"), QMessageBox::RejectRole);
  box.setDefaultButton(send);
  box.exec();

  SetEnabled(box.clickedButton() == send);
}

bool CrashReporting::IsEnabled() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return s.value(kSendReports, false).toBool();
}

void CrashReporting::SetEnabled(bool enabled) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue(kSendReports, enabled);

#ifdef HAVE_SENTRY
  if (sInitialised) {
    if (enabled) {
      sentry_user_consent_give();
    } else {
      sentry_user_consent_revoke();
    }
  }
#endif
}
