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

#include <QtGlobal>

#include <cxxabi.h>
#ifdef Q_OS_UNIX
#include <execinfo.h>
#endif

#include <QAbstractSocket>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QLocalSocket>
#include <QStringList>

#include <glib.h>

#include "logging.h"


namespace logging {

const char* kDefaultLogLevels = "GstEnginePipeline:2,*:3";

namespace {
Level sDefaultLevel = Level_Debug;
QMap<QString, Level>* sClassLevels = NULL;
QIODevice* sNullDevice = NULL;

const char* kMessageHandlerMagic = "__logging_message__";
const int kMessageHandlerMagicLength = strlen(kMessageHandlerMagic);
QtMsgHandler sOriginalMessageHandler = NULL;

QList<QIODevice*> sOutputDevices;

// QIODevice doesn't have a flush() method, but its subclasses do.  This picks
// the right method to call depending on the runtime type of the device.
void FlushDevice(QIODevice* device) {
  if (QFile* file = qobject_cast<QFile*>(device)) {
    file->flush();
  } else if (QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(device)) {
    socket->flush();
  } else if (QLocalSocket* socket = qobject_cast<QLocalSocket*>(device)) {
    socket->flush();
  }
}

} // namespace


void GLog(const char* domain, int level, const char* message, void* user_data) {
  switch (level) {
    case G_LOG_FLAG_RECURSION:
    case G_LOG_FLAG_FATAL:
    case G_LOG_LEVEL_ERROR:
    case G_LOG_LEVEL_CRITICAL: qLog(Error)   << message; break;
    case G_LOG_LEVEL_WARNING:  qLog(Warning) << message; break;
    case G_LOG_LEVEL_MESSAGE:
    case G_LOG_LEVEL_INFO:     qLog(Info)    << message; break;
    case G_LOG_LEVEL_DEBUG:
    default:                   qLog(Debug)   << message; break;
  }
}

static void MessageHandler(QtMsgType type, const char* message) {
  if (strncmp(kMessageHandlerMagic, message, kMessageHandlerMagicLength) == 0) {
    // Output to stderr.
    fprintf(stderr, "%s\n", message + kMessageHandlerMagicLength);

    // Output to all the configured output devices.
    foreach (QIODevice* device, sOutputDevices) {
      device->write(message + kMessageHandlerMagicLength);
      device->write("\n", 1);
      FlushDevice(device);
    }

    return;
  }

  Level level = Level_Debug;
  switch (type) {
    case QtFatalMsg:
    case QtCriticalMsg: level = Level_Error;   break;
    case QtWarningMsg:  level = Level_Warning; break;
    case QtDebugMsg:
    default:            level = Level_Debug;   break;
  }

  foreach (const QString& line, QString::fromLocal8Bit(message).split('\n')) {
    CreateLogger(level, "unknown", -1) << line.toLocal8Bit().constData();
  }

  if (type == QtFatalMsg) {
    abort();
  }
}


void Init() {
  delete sClassLevels;
  delete sNullDevice;

  sClassLevels = new QMap<QString, Level>();
  sNullDevice = new NullDevice;

  // Catch other messages from Qt
  if (!sOriginalMessageHandler) {
    sOriginalMessageHandler = qInstallMsgHandler(MessageHandler);
  }
}

void AddOutputDevice(QIODevice* device) {
  sOutputDevices << device;
}

void SetLevels(const QString& levels) {
  if (!sClassLevels)
    return;

  foreach (const QString& item, levels.split(',')) {
    const QStringList class_level = item.split(':');

    QString class_name;
    bool ok = false;
    int level = Level_Error;

    if (class_level.count() == 1) {
      level = class_level.last().toInt(&ok);
    } else if (class_level.count() == 2) {
      class_name = class_level.first();
      level = class_level.last().toInt(&ok);
    }

    if (!ok || level < Level_Error || level > Level_Debug) {
      continue;
    }

    if (class_name.isEmpty() || class_name == "*") {
      sDefaultLevel = (Level) level;
    } else {
      sClassLevels->insert(class_name, (Level) level);
    }
  }
}

QString ParsePrettyFunction(const char * pretty_function) {
  // Get the class name out of the function name.
  QString class_name = pretty_function;
  const int paren = class_name.indexOf('(');
  if (paren != -1) {
    const int colons = class_name.lastIndexOf("::", paren);
    if (colons != -1) {
      class_name = class_name.left(colons);
    } else {
      class_name = class_name.left(paren);
    }
  }

  const int space = class_name.lastIndexOf(' ');
  if (space != -1) {
    class_name = class_name.mid(space+1);
  }

  return class_name;
}

QDebug CreateLogger(Level level, const QString& class_name, int line) {
  // Map the level to a string
  const char* level_name = NULL;
  switch (level) {
    case Level_Debug:   level_name = " DEBUG "; break;
    case Level_Info:    level_name = " INFO  "; break;
    case Level_Warning: level_name = " WARN  "; break;
    case Level_Error:   level_name = " ERROR "; break;
  }

  // Check the settings to see if we're meant to show or hide this message.
  Level threshold_level = sDefaultLevel;
  if (sClassLevels && sClassLevels->contains(class_name)) {
    threshold_level = sClassLevels->value(class_name);
  }

  if (level > threshold_level) {
    return QDebug(sNullDevice);
  }

  QString function_line = class_name;
  if (line != -1) {
    function_line += ":" + QString::number(line);
  }

  QDebug ret(QtDebugMsg);
  ret.nospace() << kMessageHandlerMagic
      << QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toAscii().constData()
      << level_name << function_line.leftJustified(32).toAscii().constData();

  return ret.space();
}

QString CXXDemangle(const QString& mangled_function) {
  int status;
  char* demangled_function = abi::__cxa_demangle(
      mangled_function.toAscii().constData(),
      NULL,
      NULL,
      &status);
  if (status == 0) {
    QString ret = QString::fromAscii(demangled_function);
    free(demangled_function);
    return ret;
  }
  return mangled_function;  // Probably not a C++ function.
}

QString DarwinDemangle(const QString& symbol) {
  QStringList split = symbol.split(' ', QString::SkipEmptyParts);
  QString mangled_function = split[3];
  return CXXDemangle(mangled_function);
}

QString LinuxDemangle(const QString& symbol) {
  QRegExp regex("\\(([^+]+)");
  if (!symbol.contains(regex)) {
    return symbol;
  }
  QString mangled_function = regex.cap(1);
  return CXXDemangle(mangled_function);
}

QString DemangleSymbol(const QString& symbol) {
#ifdef Q_OS_DARWIN
  return DarwinDemangle(symbol);
#elif defined(Q_OS_LINUX)
  return LinuxDemangle(symbol);
#else
  return symbol;
#endif
}

void DumpStackTrace() {
#ifdef Q_OS_UNIX
  void* callstack[128];
  int callstack_size = backtrace(reinterpret_cast<void**>(&callstack), sizeof(callstack));
  char** symbols = backtrace_symbols(reinterpret_cast<void**>(&callstack), callstack_size);
  // Start from 1 to skip ourself.
  for (int i = 1; i < callstack_size; ++i) {
    qLog(Debug) << DemangleSymbol(QString::fromAscii(symbols[i]));
  }
  free(symbols);
#else
  qLog(Debug) << "FIXME: Implement printing stack traces on this platform";
#endif
}

} // namespace logging
