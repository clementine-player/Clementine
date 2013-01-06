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

#include <QDir>

#include "client/linux/handler/exception_handler.h"
#include "third_party/lss/linux_syscall_support.h"

const int CrashReporting::kSendCrashReportArgumentCount = 4;

namespace {

bool Handler(const google_breakpad::MinidumpDescriptor& dump,
             void* context, bool succeeded) {
  CrashReporting::Print(
      "Clementine has crashed!  A crash report has been saved to:\n  ");
  CrashReporting::Print(dump.path());
  CrashReporting::Print(
      "\n\nPlease send this to the developers so they can fix the problem:\n"
      "  http://code.google.com/p/clementine-player/issues/entry\n\n");

  if (CrashReporting::application_path()) {
    // We know the path to clementine, so exec it again to prompt the user to
    // upload the report.
    const char* argv[] = {
      CrashReporting::application_path(),
      CrashReporting::kSendCrashReportOption,
      dump.path(),
      CrashReporting::log_filename(),
      NULL
    };

    sys_execv(CrashReporting::application_path(), argv);
  }

  return true;
}

} // namespace


CrashReporting::CrashReporting()
  : handler_(new google_breakpad::ExceptionHandler(
        google_breakpad::MinidumpDescriptor(
            QDir::tempPath().toLocal8Bit().constData()),
        NULL, Handler, this, true, -1)) {
}

CrashReporting::~CrashReporting() {
}

void CrashReporting::Print(const char* message) {
  if (message) {
    sys_write(1, message, strlen(message));
  }
}

QString CrashReporting::minidump_filename_from_args(const QStringList& args) {
  return args[2];
}

QString CrashReporting::log_filename_from_args(const QStringList& args) {
  return args[3];
}
