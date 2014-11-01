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

#ifndef CORE_CRASHREPORTING_H_
#define CORE_CRASHREPORTING_H_

#include <memory>

#include <QObject>

class QFile;
class QNetworkAccessManager;
class QProgressDialog;

namespace google_breakpad {
class ExceptionHandler;
}

// Wraps google_breakpad::ExceptionHandler - while an instance of this class
// is alive crashes will be handled.
class CrashReporting {
 public:
  CrashReporting();
  ~CrashReporting();

  // If the commandline contains the --send-crash-report option, the user will
  // be prompted to send the crash report and the function will return true
  // (in which case the caller should exit the program).  Otherwise the function
  // returns false.
  static bool SendCrashReport(int argc, char** argv);

  // If this is set then the application is exec'd again with
  // --send-crash-report when a crash happens.
  static void SetApplicationPath(const QString& path);

 private:
  // Prints the message to stdout without using libc.
  static void Print(const char* message);

  // Breakpad callback.
  static bool Handler(const char* dump_path, const char* minidump_id,
                      void* context, bool succeeded);

 private:
  Q_DISABLE_COPY(CrashReporting);

  static const char* kSendCrashReportOption;
  static char* sPath;

  std::unique_ptr<google_breakpad::ExceptionHandler> handler_;
};

// Asks the user if he wants to send a crash report, and displays a progress
// dialog while uploading it if he does.
class CrashSender : public QObject {
  Q_OBJECT

 public:
  explicit CrashSender(const QString& path);

  // Returns false if the user doesn't want to send the crash report (caller
  // should exit), or true if he does (caller should start the Qt event loop).
  bool Start();

 private slots:
  void RedirectFinished();
  void UploadProgress(qint64 bytes);

 private:
  static const char* kUploadURL;

  QNetworkAccessManager* network_;

  QString path_;
  QFile* file_;
  QProgressDialog* progress_;
};

#endif  // CORE_CRASHREPORTING_H_
