#include "mainwindow.h"
#include "directory.h"
#include "song.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QCoreApplication::setApplicationName("Tangerine");
  QCoreApplication::setOrganizationName("Tangerine");
  QCoreApplication::setOrganizationDomain("davidsansome.com");

  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<SongList>("SongList");

  QApplication a(argc, argv);

  MainWindow w;

  return a.exec();
}
