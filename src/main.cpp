#include "mainwindow.h"
#include "directory.h"
#include "song.h"

#include <QtSingleApplication>
#include <QtDebug>

int main(int argc, char *argv[]) {
  QCoreApplication::setApplicationName("Clementine");
  QCoreApplication::setApplicationVersion("0.1");
  QCoreApplication::setOrganizationName("Clementine");
  QCoreApplication::setOrganizationDomain("davidsansome.com");

  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<SongList>("SongList");

  QtSingleApplication a(argc, argv);

  if (a.isRunning()) {
    qDebug() << "Clementine is already running - activating existing window";
    if (a.sendMessage("wake up!"))
      return 0;
    // Couldn't send the message so start anyway
  }

  MainWindow w;
  a.setActivationWindow(&w);

  QObject::connect(&a, SIGNAL(messageReceived(QString)), &w, SLOT(show()));

  return a.exec();
}
