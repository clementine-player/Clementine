/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

// airplay-poc: plays audio on an AirPlay receiver.
//
//   airplay-poc list
//   airplay-poc play <device name | host[:port]> [--tone | --wav FILE | -]
//                    [--v1 | --v2] [--pin PIN] [--volume 0-100] [--seconds N]
//
// "-" reads raw S16LE 44.1 kHz stereo from stdin.

#include <QCoreApplication>
#include <QFile>
#include <QSysInfo>
#include <QTextStream>
#include <cstdio>

#include "audiosource.h"
#include "discovery.h"
#include "logging.h"
#include "session.h"

using airplay::Device;
using airplay::Discovery;
using airplay::Session;
using airplay::Version;

namespace {

const int kBrowseMsec = 3000;

QTextStream& Out() {
  static QTextStream out(stdout);
  return out;
}

int Usage() {
  QTextStream(stderr)
      << "usage: airplay-poc list | play <device|host[:port]> "
         "[--tone|--wav FILE|-] [--v1|--v2] [--pin PIN] [--volume N] "
         "[--seconds N] [--name NAME] [--device-id HEX]\n";
  return 2;
}

void Fail(const QString& error) {
  QTextStream(stderr) << "Error: " << error << "\n";
  QCoreApplication::exit(1);
}

void List() {
  Discovery* discovery = new Discovery(qApp);
  QObject::connect(
      discovery, &Discovery::Finished, [](const QList<Device>& devices) {
        for (const Device& d : devices) {
          QStringList addresses;
          for (const QHostAddress& a : d.addresses) addresses << a.toString();
          Out() << d.name << "\n  addresses [" << addresses.join(", ")
                << "], raop port " << d.raop_port << ", airplay port "
                << d.airplay_port << ", " << airplay::VersionName(d.version())
                << ", features 0x" << QString::number(d.features(), 16)
                << (d.password_required() ? ", password required" : "") << "\n";
          for (auto it = d.txt.begin(); it != d.txt.end(); ++it) {
            Out() << "    " << it.key() << "=" << it.value() << "\n";
          }
        }
        Out().flush();
        QCoreApplication::exit(0);
      });
  discovery->Browse(kBrowseMsec);
}

struct PlayArgs {
  QString target;
  bool version_set = false;
  Session::Options options;
  qint64 seconds = -1;
  QString input = "--tone";
  QString wav;
};

void StartSession(const PlayArgs& args, const QHostAddress& host,
                  quint16 port) {
  const qint64 frames =
      args.seconds < 0 ? -1 : args.seconds * airplay::kSampleRate;
  std::unique_ptr<airplay::AudioSource> source;
  if (args.input == "--tone") {
    source.reset(new airplay::ToneSource(frames < 0 ? 10 * airplay::kSampleRate
                                                    : frames));
  } else if (args.input == "--wav") {
    QString error;
    std::unique_ptr<QIODevice> file = airplay::OpenWav(args.wav, &error);
    if (!file) {
      Fail(error);
      return;
    }
    source.reset(new airplay::DeviceSource(std::move(file), frames));
  } else {
    std::unique_ptr<QFile> in(new QFile);
    if (!in->open(stdin, QIODevice::ReadOnly)) {
      Fail("couldn't read stdin");
      return;
    }
    source.reset(new airplay::DeviceSource(std::move(in), frames));
  }

  Session* session = new Session(args.options, std::move(source), qApp);
  QObject::connect(session, &Session::Finished,
                   []() { QCoreApplication::exit(0); });
  QObject::connect(session, &Session::Failed, &Fail);
  session->Start(host, port);
}

void Play(PlayArgs args) {
  // A host or host:port, else a name to look up.
  QHostAddress host;
  if (host.setAddress(args.target)) {
    StartSession(args, host, 7000);
    return;
  }
  const int colon = args.target.lastIndexOf(':');
  bool port_ok = false;
  const quint16 port = args.target.mid(colon + 1).toUShort(&port_ok);
  if (colon > 0 && port_ok && host.setAddress(args.target.left(colon))) {
    StartSession(args, host, port);
    return;
  }

  qCInfo(airplay_log) << "Looking for" << qUtf8Printable(args.target);
  Discovery* discovery = new Discovery(qApp);
  QObject::connect(
      discovery, &Discovery::Finished, [args](const QList<Device>& devices) {
        const Device* found = nullptr;
        for (const Device& d : devices) {
          if (d.name.compare(args.target, Qt::CaseInsensitive) == 0) {
            found = &d;
          }
        }
        for (const Device& d : devices) {
          if (!found && d.name.contains(args.target, Qt::CaseInsensitive)) {
            found = &d;
          }
        }
        if (!found) {
          Fail(QString("no AirPlay device called %1; try `airplay-poc list`")
                   .arg(args.target));
          return;
        }
        if (found->address().isNull() || !found->port()) {
          Fail(found->name + " has no address or port");
          return;
        }
        qCInfo(airplay_log).noquote() << QString("Found %1 with features 0x%2")
                                             .arg(found->name)
                                             .arg(found->features(), 0, 16);
        if (found->password_required() && !args.options.has_password) {
          Fail(found->name +
               " requires a password: pass --pin or set AIRPLAY_PIN");
          return;
        }
        PlayArgs a = args;
        if (!a.version_set) a.options.version = found->version();
        StartSession(a, found->address(), found->port());
      });
  discovery->Browse(kBrowseMsec);
}

}  // namespace

int main(int argc, char** argv) {
  // Qt logs to the journal when stderr isn't a terminal.
  qputenv("QT_FORCE_STDERR_LOGGING", "1");
  QCoreApplication app(argc, argv);
  qSetMessagePattern(
      "[%{time yyyy-MM-ddThh:mm:ss.zzz} "
      "%{if-debug}DEBUG%{endif}%{if-info}INFO %{endif}%{if-warning}WARN "
      "%{endif}%{if-critical}ERROR%{endif}] %{message}");
  if (!qEnvironmentVariableIsEmpty("AIRPLAY_DEBUG")) {
    QLoggingCategory::setFilterRules("airplay.debug=true");
  }

  const QStringList args = app.arguments().mid(1);
  if (args.value(0) == "list") {
    List();
  } else if (args.value(0) == "play" && args.size() >= 2) {
    PlayArgs play;
    play.target = args[1];
    play.options.name = "Clementine on " + QSysInfo::machineHostName();
    for (int i = 2; i < args.size(); ++i) {
      const QString& arg = args[i];
      const bool has_value = i + 1 < args.size();
      if (arg == "--v1" || arg == "--v2") {
        play.version_set = true;
        play.options.version =
            arg == "--v1" ? Version::kAirPlay1 : Version::kAirPlay2;
      } else if (arg == "--device-id" && has_value) {
        play.options.device_id = args[++i].toUpper().toLatin1();
      } else if (arg == "--name" && has_value) {
        play.options.name = args[++i];
      } else if (arg == "--pin" && has_value) {
        play.options.pin = args[++i];
        play.options.has_password = true;
      } else if (arg == "--volume" && has_value) {
        play.options.volume = args[++i].toInt();
      } else if (arg == "--seconds" && has_value) {
        play.seconds = args[++i].toLongLong();
      } else if (arg == "--wav" && has_value) {
        play.input = arg;
        play.wav = args[++i];
      } else if (arg == "--tone" || arg == "-") {
        play.input = arg;
      } else {
        return Usage();
      }
    }
    if (!qEnvironmentVariableIsEmpty("AIRPLAY_PIN")) {
      play.options.pin = qEnvironmentVariable("AIRPLAY_PIN");
      play.options.has_password = true;
    }
    Play(play);
  } else {
    return Usage();
  }
  return app.exec();
}
