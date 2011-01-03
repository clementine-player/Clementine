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

#include <QtGlobal>

#ifdef Q_OS_WIN32
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#  include <iostream>
#endif // Q_OS_WIN32

#include "config.h"
#include "core/commandlineoptions.h"
#include "core/database.h"
#include "core/encoding.h"
#include "core/mac_startup.h"
#include "core/network.h"
#include "core/networkproxyfactory.h"
#include "core/player.h"
#include "core/potranslator.h"
#include "core/song.h"
#include "core/utilities.h"
#include "engines/enginebase.h"
#include "library/directory.h"
#include "playlist/playlist.h"
#include "remote/httpserver.h"
#include "remote/zeroconf.h"
#include "smartplaylists/generator.h"
#include "ui/equalizer.h"
#include "ui/iconloader.h"
#include "ui/mainwindow.h"

#include "qtsingleapplication.h"
#include "qtsinglecoreapplication.h"

#include <QDir>
#include <QLibraryInfo>
#include <QNetworkProxyFactory>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTranslator>
#include <QtDebug>

#include <glib-object.h>
#include <glib/gutils.h>
#include <gst/gst.h>

#include <echonest/Config.h>

#ifdef Q_OS_DARWIN
  #include <sys/resource.h>
  #include <sys/sysctl.h>
#endif

#ifdef HAVE_LIBLASTFM
  #include "radio/lastfmservice.h"
#endif

class GstEnginePipeline;

// Load sqlite plugin on windows and mac.
#ifdef HAVE_STATIC_SQLITE
# include <QtPlugin>
  Q_IMPORT_PLUGIN(qsqlite)
#endif

void LoadTranslation(const QString& prefix, const QString& path,
                     const QString& override_language = QString()) {
#if QT_VERSION < 0x040700
  // QTranslator::load will try to open and read "clementine" if it exists,
  // without checking if it's a file first.
  // This was fixed in Qt 4.7
  QFileInfo maybe_clementine_directory(path + "/clementine");
  if (maybe_clementine_directory.exists() && !maybe_clementine_directory.isFile())
    return;
#endif

  QString language = override_language.isEmpty() ?
                     QLocale::system().name() : override_language;

  QTranslator* t = new PoTranslator;
  if (t->load(prefix + "_" + language, path))
    QCoreApplication::installTranslator(t);
  else
    delete t;
}

void GLog(const gchar* domain,
          GLogLevelFlags level,
          const gchar* message,
          gpointer user_data) {
  qDebug() << "GLOG" << message;
}

int main(int argc, char *argv[]) {
#ifdef Q_OS_DARWIN
  // Do Mac specific startup to get media keys working.
  // This must go before QApplication initialisation.
  mac::MacMain();

  {
    // Bump the soft limit for the number of file descriptors from the default of 256 to
    // the maximum (usually 10240).
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);

    // getrlimit() lies about the hard limit so we have to check sysctl.
    int max_fd = 0;
    size_t len = sizeof(max_fd);
    sysctlbyname("kern.maxfilesperproc", &max_fd, &len, NULL, 0);

    limit.rlim_cur = max_fd;
    int ret = setrlimit(RLIMIT_NOFILE, &limit);

    if (ret == 0) {
      qDebug() << "Max fd:" << max_fd;
    }
  }
#endif

  QCoreApplication::setApplicationName("Clementine");
  QCoreApplication::setApplicationVersion(CLEMENTINE_VERSION_DISPLAY);
  QCoreApplication::setOrganizationName("Clementine");
  QCoreApplication::setOrganizationDomain("davidsansome.com");

#ifdef Q_OS_DARWIN
  // Must happen after QCoreApplication::setOrganizationName().
  if (mac::MigrateLegacyConfigFiles()) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(Utilities::GetConfigPath(
        Utilities::Path_Root) + "/" + Database::kDatabaseFilename);
    db.open();
    QSqlQuery query(
        "UPDATE songs SET art_manual = replace("
        "art_manual, '.config', 'Library/Application Support') "
        "WHERE art_manual LIKE '%.config%'", db);
    query.exec();
    db.close();
    QSqlDatabase::removeDatabase(db.connectionName());
  }
#endif

  // This makes us show up nicely in gnome-volume-control
  g_type_init();
  g_set_application_name(QCoreApplication::applicationName().toLocal8Bit());

  g_log_set_default_handler(&GLog, NULL);

  qRegisterMetaType<Directory>("Directory");
  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<Subdirectory>("Subdirectory");
  qRegisterMetaType<SubdirectoryList>("SubdirectoryList");
  qRegisterMetaType<SongList>("SongList");
  qRegisterMetaType<PlaylistItemList>("PlaylistItemList");
  qRegisterMetaType<Engine::State>("Engine::State");
  qRegisterMetaType<Engine::SimpleMetaBundle>("Engine::SimpleMetaBundle");
  qRegisterMetaType<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaTypeStreamOperators<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaType<const char*>("const char*");
  qRegisterMetaType<QNetworkReply*>("QNetworkReply*");
  qRegisterMetaType<QNetworkReply**>("QNetworkReply**");
  qRegisterMetaType<smart_playlists::GeneratorPtr>("smart_playlists::GeneratorPtr");
  qRegisterMetaType<ColumnAlignmentMap>("ColumnAlignmentMap");
  qRegisterMetaTypeStreamOperators<QMap<int, int> >("ColumnAlignmentMap");

  qRegisterMetaType<GstBuffer*>("GstBuffer*");
  qRegisterMetaType<GstElement*>("GstElement*");
  qRegisterMetaType<GstEnginePipeline*>("GstEnginePipeline*");

#ifdef HAVE_LIBLASTFM
  lastfm::ws::ApiKey = LastFMService::kApiKey;
  lastfm::ws::SharedSecret = LastFMService::kSecret;
#endif

  CommandlineOptions options(argc, argv);

  {
    // Only start a core application now so we can check if there's another
    // Clementine running without needing an X server.
    // This MUST be done before parsing the commandline options so QTextCodec
    // gets the right system locale for filenames.
    QtSingleCoreApplication a(argc, argv);

    // Parse commandline options - need to do this before starting the
    // full QApplication so it works without an X server
    if (!options.Parse())
      return 1;

    if (a.isRunning()) {
      if (options.is_empty()) {
        qDebug() << "Clementine is already running - activating existing window";
      }
      if (a.sendMessage(options.Serialize(), 5000)) {
        return 0;
      }
      // Couldn't send the message so start anyway
    }
  }

  // Detect technically invalid usage of non-ASCII in ID3v1 tags.
  UniversalEncodingHandler handler;
  TagLib::ID3v1::Tag::setStringHandler(&handler);

  QtSingleApplication a(argc, argv);
#ifdef Q_OS_DARWIN
  QCoreApplication::setLibraryPaths(
      QStringList() << QCoreApplication::applicationDirPath() + "/../PlugIns");
#endif

  a.setQuitOnLastWindowClosed(false);

  // Do this check again because another instance might have started by now
  if (a.isRunning() && a.sendMessage(options.Serialize(), 5000)) {
    return 0;
  }

#ifndef Q_OS_DARWIN
  // Gnome on Ubuntu has menu icons disabled by default.  I think that's a bad
  // idea, and makes some menus in Clementine look confusing.
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
#else
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

  // Resources
  Q_INIT_RESOURCE(data);
  Q_INIT_RESOURCE(translations);

  // Has the user forced a different language?
  QString language = options.language();
  if (language.isEmpty()) {
    QSettings s;
    s.beginGroup("General");
    language = s.value("language").toString();
  }

  // Translations
  LoadTranslation("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath), language);
  LoadTranslation("clementine", ":/translations", language);
  LoadTranslation("clementine", a.applicationDirPath(), language);
  LoadTranslation("clementine", QDir::currentPath(), language);

  // Icons
  IconLoader::Init();

  Echonest::Config::instance()->setAPIKey("DFLFLJBUF4EGTXHIG");
  Echonest::Config::instance()->setNetworkAccessManager(new NetworkAccessManager);

  // Network proxy
  QNetworkProxyFactory::setApplicationProxyFactory(
      NetworkProxyFactory::Instance());

  // Seed the random number generator
  srand(time(NULL));

  Zeroconf* zeroconf = Zeroconf::GetZeroconf();
  if (zeroconf) {
    HttpServer* server = new HttpServer;
    server->Listen(QHostAddress::Any, 12345);

    zeroconf->Publish("local", "_clementine._tcp", "Clementine", 12345);
  }

  // Window
  MainWindow w;

  QObject::connect(&a, SIGNAL(messageReceived(QByteArray)), &w, SLOT(CommandlineOptionsReceived(QByteArray)));
  w.CommandlineOptionsReceived(options);

  return a.exec();
}
