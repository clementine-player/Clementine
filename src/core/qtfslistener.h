#ifndef QTFSLISTENER_H
#define QTFSLISTENER_H

#include "filesystemwatcherinterface.h"

#include <QFileSystemWatcher>

class QtFSListener : public FileSystemWatcherInterface {
  Q_OBJECT
 public:
  QtFSListener(QObject* parent);
  virtual void AddPath(const QString& path);

 private:
  QFileSystemWatcher watcher_;

};

#endif
