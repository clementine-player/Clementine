#include "qtfslistener.h"

QtFSListener::QtFSListener(QObject* parent)
    : FileSystemWatcherInterface(parent),
      watcher_(this) {
  connect(&watcher_, SIGNAL(directoryChanged(const QString&)),
      SIGNAL(PathChanged(const QString&)));
}

void QtFSListener::AddPath(const QString& path) {
  watcher_.addPath(path);
}
