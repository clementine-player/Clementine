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
#include "crashsender.h"

#include <QApplication>


const char* CrashReporting::kSendCrashReportOption = "--send-crash-report";
char* CrashReporting::sPath = NULL;
char* CrashReporting::sLogFilename = NULL;


bool CrashReporting::SendCrashReport(int argc, char** argv) {
#ifdef HAVE_BREAKPAD
  if (argc != 4 || strcmp(argv[1], kSendCrashReportOption) != 0) {
    return false;
  }

  QApplication a(argc, argv);

  CrashSender sender(argv[2], argv[3]);
  if (sender.Start()) {
    a.exec();
  }

  return true;
#else // HAVE_BREAKPAD
  return false;
#endif
}

void CrashReporting::SetApplicationPath(const QString& path) {
  sPath = strdup(path.toLocal8Bit().constData());
}

void CrashReporting::SetLogFilename(const QString& path) {
  sLogFilename = strdup(path.toLocal8Bit().constData());
}
