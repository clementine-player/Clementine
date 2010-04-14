/* This file is part of Clementine.

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

#ifdef Q_OS_WIN32
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#  include <iostream>
#endif // Q_OS_WIN32

#include "directory.h"
#include "lastfmservice.h"
#include "mainwindow.h"
#include "player.h"
#include "song.h"
#include "equalizer.h"
#include "potranslator.h"
#include "commandlineoptions.h"

#include <QtSingleApplication>
#include <QtDebug>
#include <QLibraryInfo>
#include <QTranslator>
#include <QDir>
#include <QNetworkAccessManager>

#ifdef Q_WS_X11
#  include <QDBusConnection>
#  include <QDBusMetaType>
#  include "mpris.h"
#  include "osd.h"
#endif

#include "mac_startup.h"

// Load sqlite plugin on windows and mac.
#ifndef Q_WS_X11
# include <QtPlugin>
  Q_IMPORT_PLUGIN(qsqlite)
#endif

void LoadTranslation(const QString& prefix, const QString& path) {
#if QT_VERSION < 0x040700
  // QTranslator::load will try to open and read "clementine" if it exists,
  // without checking if it's a file first.
  // This was fixed in Qt 4.7
  QFileInfo maybe_clementine_directory(path + "/clementine");
  if (maybe_clementine_directory.exists() && !maybe_clementine_directory.isFile())
    return;
#endif

  QTranslator* t = new PoTranslator;
  if (t->load(prefix + "_" + QLocale::system().name(), path))
    QCoreApplication::installTranslator(t);
  else
    delete t;
}

int main(int argc, char *argv[]) {
#ifdef Q_OS_DARWIN
  // Do Mac specific startup to get media keys working.
  // This must go before QApplication initialisation.
  mac::MacMain();
#endif

  QCoreApplication::setApplicationName("Clementine");
  QCoreApplication::setApplicationVersion("0.2");
  QCoreApplication::setOrganizationName("Clementine");
  QCoreApplication::setOrganizationDomain("davidsansome.com");

  qRegisterMetaType<Directory>("Directory");
  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<Subdirectory>("Subdirectory");
  qRegisterMetaType<SubdirectoryList>("SubdirectoryList");
  qRegisterMetaType<SongList>("SongList");
  qRegisterMetaType<Engine::State>("Engine::State");
  qRegisterMetaType<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaTypeStreamOperators<Equalizer::Params>("Equalizer::Params");


  lastfm::ws::ApiKey = LastFMService::kApiKey;
  lastfm::ws::SharedSecret = LastFMService::kSecret;

  QtSingleApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);

  // Resources
  Q_INIT_RESOURCE(data);
  Q_INIT_RESOURCE(translations);

  // Translations
  LoadTranslation("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  LoadTranslation("clementine", ":/translations");
  LoadTranslation("clementine", a.applicationDirPath());
  LoadTranslation("clementine", QDir::currentPath());


  CommandlineOptions options(argc, argv);
  if (!options.Parse())
    return 1;

  if (a.isRunning()) {
    if (options.is_empty())
      qDebug() << "Clementine is already running - activating existing window";
    if (a.sendMessage(options.Serialize()))
      return 0;
    // Couldn't send the message so start anyway
  }

  QNetworkAccessManager network;

  // MPRIS DBus interface.
#ifdef Q_WS_X11
  qDBusRegisterMetaType<DBusStatus>();
  qDBusRegisterMetaType<Version>();
  qDBusRegisterMetaType<QImage>();
  QDBusConnection::sessionBus().registerService("org.mpris.clementine");
  MPRIS mpris;
#endif

  #ifdef Q_OS_DARWIN
  QString scanner_path = QCoreApplication::applicationDirPath() + "/../PlugIns/gst-plugin-scanner";
  QString plugin_path = QCoreApplication::applicationDirPath() + "/../PlugIns/gstreamer";
  setenv("GST_PLUGIN_SCANNER", scanner_path.toAscii().constData(), 1);
  setenv("GST_PLUGIN_PATH", plugin_path.toAscii().constData(), 1);
  #endif

  // Window
  MainWindow w(&network);

#ifdef Q_OS_DARWIN
  mac::SetApplicationHandler(&w);
#endif

  QObject::connect(&a, SIGNAL(messageReceived(QByteArray)), &w, SLOT(CommandlineOptionsReceived(QByteArray)));
  w.CommandlineOptionsReceived(options);

  return a.exec();
}
