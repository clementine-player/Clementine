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

#include <QList>
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
// A report is sent with what the current run knows, not what the crashed one
// did, so anything Sentry should know about the crash has to be kept on disk
// beside its minidump until then. Crashes made with --crash are kept apart
// that way, and sent tagged with kCrashTypeTag set to kCrashTypeForced.
//
// Builds without ENABLE_SENTRY, or with an empty SENTRY_DSN, get no-op stubs.
class CrashReporting {
 public:
  static const char* kSettingsGroup;
  static const char* kAskAfterCrash;

  // The Sentry tag, and its value, that crashes made with --crash have.
  static const char* kCrashTypeTag;
  static const char* kCrashTypeForced;

  enum class CrashType { kUnexpected, kForced };

  // A crash's minidump, waiting for the user to say whether to send it.
  struct PendingReport {
    QString minidump_path;
    CrashType type;
  };

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

  // Crashes Clementine on purpose, for --crash. It always crashes here, with a
  // write to a null pointer, and the report is tagged as forced (see
  // kCrashTypeTag), so the reports it makes can be told apart in Sentry.
  [[noreturn]] static void CrashForTesting();

  // The steps that keep reports in database_path, the directory given to
  // Crashpad, for the user to be asked about. Public for tests.
  //
  // Records that the next crash is a forced one.
  static void MarkNextCrashForced(const QString& database_path);
  // Moves the minidumps Crashpad saved since the last start out of its
  // database, where sentry_init() would delete them after two days, keeping
  // apart the forced ones. Run before sentry_init(), while no handler is using
  // the database.
  static void CollectNewReports(const QString& database_path);
  // The reports that CollectNewReports() has kept so far.
  static QList<PendingReport> PendingReports(const QString& database_path);

 private:
  Q_DISABLE_COPY(CrashReporting)

  static QString DatabasePath();

  static bool sInitialised;
};

#endif  // CORE_CRASHREPORTING_H_
