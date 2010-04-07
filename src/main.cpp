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

// Load sqlite plugin on windows
#ifdef WIN32
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

  QTranslator* t = new QTranslator;
  if (t->load(prefix + "_" + QLocale::system().name(), path))
    QCoreApplication::installTranslator(t);
  else
    delete t;
}

int main(int argc, char *argv[]) {
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


  lastfm::ws::ApiKey = LastFMService::kApiKey;
  lastfm::ws::SharedSecret = LastFMService::kSecret;

  QtSingleApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);

  if (a.isRunning()) {
    qDebug() << "Clementine is already running - activating existing window";
    if (a.sendMessage("wake up!"))
      return 0;
    // Couldn't send the message so start anyway
  }

  // Resources
  Q_INIT_RESOURCE(data);
  Q_INIT_RESOURCE(translations);

  // Translations
  LoadTranslation("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  LoadTranslation("clementine", ":/translations");
  LoadTranslation("clementine", a.applicationDirPath());
  LoadTranslation("clementine", QDir::currentPath());

  QNetworkAccessManager network;

  // MPRIS DBus interface.
#ifdef Q_WS_X11
  qDBusRegisterMetaType<DBusStatus>();
  qDBusRegisterMetaType<Version>();
  qDBusRegisterMetaType<QImage>();
  QDBusConnection::sessionBus().registerService("org.mpris.clementine");
  MPRIS mpris;
#endif

  // Window
  MainWindow w(&network);;
  a.setActivationWindow(&w);

  QObject::connect(&a, SIGNAL(messageReceived(QString)), &w, SLOT(show()));

  return a.exec();
}
