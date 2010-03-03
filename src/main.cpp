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
#include <QDir>
#include <QNetworkAccessManager>

void LoadTranslation(const QString& prefix, const QString& path) {
  QTranslator* t = new QTranslator;
  t->load(prefix + "_" + QLocale::system().name(), path);
  QCoreApplication::installTranslator(t);
}

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
  LoadTranslation("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  LoadTranslation("clementine", ":/translations");
  LoadTranslation("clementine", a.applicationDirPath());
  LoadTranslation("clementine", QDir::currentPath());

  QNetworkAccessManager network;

  // Window
  MainWindow w(&network);;
  a.setActivationWindow(&w);

  QObject::connect(&a, SIGNAL(messageReceived(QString)), &w, SLOT(show()));

  return a.exec();
}
