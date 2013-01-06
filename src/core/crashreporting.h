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

#ifndef CRASHREPORTING_H
#define CRASHREPORTING_H

#include <QString>
#include <QStringList>

#include <boost/scoped_ptr.hpp>

namespace google_breakpad {
  class ExceptionHandler;
}


// Wraps google_breakpad::ExceptionHandler - while an instance of this class
// is alive crashes will be handled.
class CrashReporting {
public:
  CrashReporting();
  ~CrashReporting();

  static const char* kSendCrashReportOption;
  static const int kSendCrashReportArgumentCount;

  // If the commandline contains the --send-crash-report option, the user will
  // be prompted to send the crash report and the function will return true
  // (in which case the caller should exit the program).  Otherwise the function
  // returns false.
  static bool SendCrashReport(int argc, char** argv);

  // If this is set then the application is exec'd again with
  // --send-crash-report when a crash happens.
  static void SetApplicationPath(const QString& path);

  // If this is set then the contents of this file is sent along with any
  // crash report.
  static void SetLogFilename(const QString& path);

  // Prints the message to stdout without using libc.
  static void Print(const char* message);

  static const char* application_path() { return sPath; }
  static const char* log_filename() { return sLogFilename; }

private:
  static QString minidump_filename_from_args(const QStringList& args);
  static QString log_filename_from_args(const QStringList& args);

private:
  Q_DISABLE_COPY(CrashReporting);

  static char* sPath;
  static char* sLogFilename;

  boost::scoped_ptr<google_breakpad::ExceptionHandler> handler_;
};


#endif // CRASHREPORTING_H
