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
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <cstdlib>

#include "config.h"
#include "core/logging.h"
#include "core/utilities.h"

#ifdef HAVE_SENTRY
#include <sentry.h>
#endif

const char* CrashReporting::kSettingsGroup = "CrashReporting";
const char* CrashReporting::kAskAfterCrash = "ask_after_crash";
const char* CrashReporting::kCrashTypeTag = "crash_type";
const char* CrashReporting::kCrashTypeForced = "forced";
bool CrashReporting::sInitialised = false;

namespace {

// Reports waiting for the user to be asked about them.
QString UnsentPath(const QString& database_path) {
  return database_path + "/unsent";
}

// The same, for forced crashes.
QString UnsentForcedPath(const QString& database_path) {
  return UnsentPath(database_path) + "/forced";
}

// Left by a run that's about to crash on purpose.
QString ForcedCrashMarkerPath(const QString& database_path) {
  return database_path + "/next-crash-forced";
}

QFileInfoList Minidumps(const QString& path) {
  return QDir(path).entryInfoList(QStringList() << "*.dmp", QDir::Files,
                                  QDir::Name);
}

}  // namespace

void CrashReporting::MarkNextCrashForced(const QString& database_path) {
  QFile marker(ForcedCrashMarkerPath(database_path));
  if (!QDir().mkpath(database_path) || !marker.open(QIODevice::WriteOnly)) {
    qLog(Warning) << "Couldn't mark the crash as forced:"
                  << marker.errorString();
  }
}

void CrashReporting::CollectNewReports(const QString& database_path) {
  // Only one run, the last one, can have crashed since the last start, so if
  // it left the marker, all of them are from the forced crash.
  QFile marker(ForcedCrashMarkerPath(database_path));
  const bool forced = marker.exists();
  QDir unsent(forced ? UnsentForcedPath(database_path)
                     : UnsentPath(database_path));

  // Crashpad keeps reports in "pending" and "completed" on Linux and macOS,
  // and all of them in "reports" on Windows.
  for (const char* subdir : {"pending", "completed", "reports"}) {
    for (const QFileInfo& info :
         Minidumps(database_path + "/" + QString::fromLatin1(subdir))) {
      if (!unsent.mkpath(".") ||
          !QFile::rename(info.absoluteFilePath(),
                         unsent.filePath(info.fileName()))) {
        qLog(Warning) << "Couldn't keep crash report"
                      << info.absoluteFilePath();
      }
    }
  }

  // Whether or not that run made a report, the next crash is a new one.
  if (forced && !marker.remove()) {
    qLog(Warning) << "Couldn't remove" << marker.fileName();
  }
}

QList<CrashReporting::PendingReport> CrashReporting::PendingReports(
    const QString& database_path) {
  QList<PendingReport> reports;
  for (const QFileInfo& info : Minidumps(UnsentPath(database_path))) {
    reports << PendingReport{info.absoluteFilePath(), CrashType::kUnexpected};
  }
  for (const QFileInfo& info : Minidumps(UnsentForcedPath(database_path))) {
    reports << PendingReport{info.absoluteFilePath(), CrashType::kForced};
  }
  return reports;
}

QString CrashReporting::DatabasePath() {
  // With the rest of Clementine's files, so portable installs keep it in their
  // own directory.
  return Utilities::GetConfigPath(Utilities::Path_Root) + "/crashreports";
}

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

sentry_uuid_t CaptureMinidump(const QString& path) {
  return sentry_capture_minidumpw(
      reinterpret_cast<const wchar_t*>(path.utf16()));
}
#else
void SetDatabasePath(sentry_options_t* options, const QString& path) {
  sentry_options_set_database_path(options,
                                   QFile::encodeName(path).constData());
}

void SetHandlerPath(sentry_options_t* options, const QString& path) {
  sentry_options_set_handler_path(options, QFile::encodeName(path).constData());
}

sentry_uuid_t CaptureMinidump(const QString& path) {
  return sentry_capture_minidump(QFile::encodeName(path).constData());
}
#endif

}  // namespace

CrashReporting::CrashReporting() {
  if (!IsAvailable()) return;

  if (!IsEnabled()) {
    // Don't leave reports from before it was turned off lying around.
    QDir(DatabasePath()).removeRecursively();
    return;
  }

  sentry_options_t* options = sentry_options_new();
  sentry_options_set_dsn(options, CLEMENTINE_SENTRY_DSN);
  sentry_options_set_release(options,
                             QString("clementine@%1")
                                 .arg(QCoreApplication::applicationVersion())
                                 .toUtf8()
                                 .constData());

  // Consent is only ever given while queueing a report the user agreed to
  // send, so the Crashpad handler never uploads anything itself: it marks
  // each report as skipped and leaves the minidump in its database.
  sentry_options_set_require_user_consent(options, 1);

  // Only crashes, no usage data.
  sentry_options_set_auto_session_tracking(options, 0);
  sentry_options_set_send_client_reports(options, 0);

  CollectNewReports(DatabasePath());
  SetDatabasePath(options, DatabasePath());

  // The macOS and Windows packages put crashpad_handler next to the
  // executable, which is where sentry-native looks by default. Linux installs
  // it in libexec: found from the executable, since a package's install prefix
  // may only have existed while it was built, or else where it was installed.
  // A build that's run from its build directory falls back to the one it was
  // built against.
  const QDir executable_dir(QCoreApplication::applicationDirPath());
  if (!QFile::exists(executable_dir.filePath(
          "crashpad_handler" CMAKE_EXECUTABLE_SUFFIX))) {
    QStringList handlers;
    if (qstrlen(CLEMENTINE_RELATIVE_CRASHPAD_HANDLER) != 0) {
      handlers << executable_dir.filePath(CLEMENTINE_RELATIVE_CRASHPAD_HANDLER);
    }
    handlers << CLEMENTINE_INSTALLED_CRASHPAD_HANDLER
             << CLEMENTINE_CRASHPAD_HANDLER;
    for (const QString& handler : handlers) {
      if (!handler.isEmpty() && QFile::exists(handler)) {
        SetHandlerPath(options, handler);
        break;
      }
    }
  }

  if (sentry_init(options) != 0) {
    qLog(Warning) << "Failed to start crash reporting";
    return;
  }
  sInitialised = true;

  // Sentry remembers consent across runs, so take back any that was left
  // given, by a run that crashed while sending a report for instance.
  sentry_user_consent_revoke();
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

void CrashReporting::AskToSendPendingReports(QWidget* parent) {
  if (!sInitialised) return;

  const QList<PendingReport> reports = PendingReports(DatabasePath());
  if (reports.isEmpty()) return;

  QMessageBox box(QMessageBox::Question,
                  QObject::tr("Clementine quit unexpectedly"),
                  QObject::tr("Clementine crashed last time it was running. "
                              "Send a crash report to the developers so they "
                              "can fix the problem?"),
                  QMessageBox::NoButton, parent);
  box.setInformativeText(
      QObject::tr("The report says what Clementine was doing when it crashed. "
                  "It can include file names and the titles of songs you "
                  "were playing. You'll be asked again each time Clementine "
                  "crashes, and you can turn this off in Preferences."));
  QPushButton* send =
      box.addButton(QObject::tr("Send report"), QMessageBox::AcceptRole);
  box.addButton(QObject::tr("Don't send"), QMessageBox::RejectRole);
  box.setDefaultButton(send);
  box.exec();

  if (box.clickedButton() == send) {
    // Consent is checked when a report is queued, not when it's uploaded, and
    // the minidump is read in when it's queued, so it's only given for long
    // enough to queue these ones.
    sentry_user_consent_give();
    for (const PendingReport& report : reports) {
      // The event is made from the scope when it's queued, so the tag is only
      // set for this one.
      if (report.type == CrashType::kForced) {
        sentry_set_tag(kCrashTypeTag, kCrashTypeForced);
      }
      CaptureMinidump(report.minidump_path);
      if (report.type == CrashType::kForced) {
        sentry_remove_tag(kCrashTypeTag);
      }
    }
    sentry_user_consent_revoke();
  }

  for (const PendingReport& report : reports) {
    QFile::remove(report.minidump_path);
  }
}

#else  // HAVE_SENTRY

CrashReporting::CrashReporting() {}

CrashReporting::~CrashReporting() {}

bool CrashReporting::IsAvailable() { return false; }

void CrashReporting::AskToSendPendingReports(QWidget*) {}

#endif  // HAVE_SENTRY

__attribute__((noinline)) void CrashReporting::CrashForTesting() {
  qLog(Error) << "Crashing on purpose because of --crash";
  if (sInitialised) MarkNextCrashForced(DatabasePath());
  // Both volatile: the pointer so the compiler can't tell it's null, and what
  // it points to so the write isn't dropped as dead before the abort() below.
  volatile int* volatile null_pointer = nullptr;
  *null_pointer = 0;
  // Not reached.
  std::abort();
}

bool CrashReporting::IsEnabled() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return s.value(kAskAfterCrash, true).toBool();
}

void CrashReporting::SetEnabled(bool enabled) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue(kAskAfterCrash, enabled);
}
