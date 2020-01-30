/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.

#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>
#include <string>

#include <QDebug>

#ifdef QT_NO_DEBUG_STREAM
#define qLog(level) \
  while (false) QNoDebug()
#else

#define qLog(level) \
  logging::CreateLogger##level(__LINE__, __PRETTY_FUNCTION__)

#define qCreateLogger(line, pretty_function, level) \
  logging::CreateLogger(logging::Level_##level,     \
                        logging::ParsePrettyFunction(pretty_function), line)

#endif // QT_NO_DEBUG_STREAM

namespace logging {
class NullDevice : public QIODevice {
 protected:
  qint64 readData(char*, qint64) { return -1; }
  qint64 writeData(const char*, qint64 len) { return len; }
};

enum Level {
  Level_Fatal = -1,
  Level_Error = 0,
  Level_Warning,
  Level_Info,
  Level_Debug,
};

void Init();
void SetLevels(const QString& levels);

void DumpStackTrace();

QString ParsePrettyFunction(const char* pretty_function);
QDebug CreateLogger(Level level, const QString& class_name, int line);

QDebug CreateLoggerFatal(int line, const char* pretty_function);
QDebug CreateLoggerError(int line, const char* pretty_function);

#ifdef QT_NO_WARNING_OUTPUT
QNoDebug CreateLoggerWarning(int, const char*);
#else
QDebug CreateLoggerWarning(int line, const char* pretty_function);
#endif // QT_NO_WARNING_OUTPUT

#ifdef QT_NO_DEBUG_OUTPUT
QNoDebug CreateLoggerInfo(int, const char*);
QNoDebug CreateLoggerDebug(int, const char*);
#else
QDebug CreateLoggerInfo(int line, const char* pretty_function);
QDebug CreateLoggerDebug(int line, const char* pretty_function);
#endif // QT_NO_DEBUG_OUTPUT


void GLog(const char* domain, int level, const char* message, void* user_data);

extern const char* kDefaultLogLevels;
}

QDebug operator<<(QDebug debug, std::chrono::seconds secs);

#endif  // LOGGING_H
