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

#include "config.h"
#include "crashreporting.h"
#include "core/logging.h"

#include <sys/syscall.h>

#include <QDir>

#include "client/mac/handler/exception_handler.h"

const int CrashReporting::kSendCrashReportArgumentCount = 5;

namespace {

bool Handler(const char* dump_path, const char* minidump_id,
             void* context, bool succeeded) {
  CrashReporting::Print(
      "Clementine has crashed!  A crash report has been saved to:\n  ");
  CrashReporting::Print(dump_path);
  CrashReporting::Print("/");
  CrashReporting::Print(minidump_id);
  CrashReporting::Print(
      ".dmp\n\n"
      "Please send this to the developers so they can fix the problem:\n"
      "  http://code.google.com/p/clementine-player/issues/entry\n\n");

  if (CrashReporting::application_path()) {
    // We know the path to clementine, so exec it again to prompt the user to
    // upload the report.
    const char* argv[] = {
      CrashReporting::application_path(),
      CrashReporting::kSendCrashReportOption,
      dump_path,
      minidump_id,
      CrashReporting::log_filename(),
      NULL
    };

    syscall(SYS_execve, CrashReporting::application_path(), argv, NULL);
  }

  return true;
}

} // namespace


CrashReporting::CrashReporting()
  : handler_(new google_breakpad::ExceptionHandler(
        QDir::tempPath().toLocal8Bit().constData(), NULL,
        Handler, this, true, NULL)) {
}

CrashReporting::~CrashReporting() {
}

void CrashReporting::Print(const char* message) {
  if (message) {
    syscall(SYS_write, 1, message, strlen(message));
  }
}

QString CrashReporting::minidump_filename_from_args(const QStringList& args) {
  return args[2] + "/" + args[3] + ".dmp";
}

QString CrashReporting::log_filename_from_args(const QStringList& args) {
  return args[4];
}
