#ifndef QTFSLISTENER_H
#define QTFSLISTENER_H

#include "filesystemwatcherinterface.h"

#include <QFileSystemWatcher>

class QtFSListener : public FileSystemWatcherInterface {
  Q_OBJECT
 public:
  QtFSListener(QObject* parent);
  virtual void AddPath(const QString& path);
  virtual void Clear();

 private:
  QFileSystemWatcher watcher_;

};

#endif
