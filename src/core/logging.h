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

#ifndef LOGGING_H
#define LOGGING_H

#include <QDebug>

#ifdef QT_NO_DEBUG_STREAM
#  define qLog(level) while (false) QNoDebug()
#else
#  define qLog(level) \
     logging::CreateLogger(logging::Level_##level, __PRETTY_FUNCTION__, __LINE__)
#endif

namespace logging {
  class NullDevice : public QIODevice {
  protected:
    qint64 readData(char*, qint64) { return -1; }
    qint64 writeData(const char*, qint64 len) { return len; }
  };

  enum Level {
    Level_Error = 0,
    Level_Warning,
    Level_Info,
    Level_Debug,
  };

  void Init();
  void SetLevels(const QString& levels);

  QDebug CreateLogger(Level level, const char* pretty_function, int line);

  extern const char* kDefaultLogLevels;
}

#endif // LOGGING_H
