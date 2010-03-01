#ifdef Q_OS_WIN32
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#  include <iostream>
#endif // Q_OS_WIN32

#include "directory.h"
#include "lastfmservice.h"
#include "mainwindow.h"
#include "song.h"

#include <QtSingleApplication>
#include <QtDebug>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char *argv[]) {
  QCoreApplication::setApplicationName("Clementine");
  QCoreApplication::setApplicationVersion("0.1");
  QCoreApplication::setOrganizationName("Clementine");
  QCoreApplication::setOrganizationDomain("davidsansome.com");

  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<SongList>("SongList");

  lastfm::ws::ApiKey = LastFMService::kApiKey;
  lastfm::ws::SharedSecret = LastFMService::kSecret;

  QtSingleApplication a(argc, argv);

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
  QTranslator qt_translator;
  qt_translator.load("qt_" + QLocale::system().name(),
       QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  a.installTranslator(&qt_translator);

  QTranslator clementine_translator;
  clementine_translator.load("clementine_" + QLocale::system().name(), ":/translations");
  a.installTranslator(&clementine_translator);

  // Window
  MainWindow w;
  a.setActivationWindow(&w);

  QObject::connect(&a, SIGNAL(messageReceived(QString)), &w, SLOT(show()));

  return a.exec();
}
