#include "filesystemwatcherinterface.h"

#include "qtfslistener.h"

#ifdef Q_OS_DARWIN
#include "macfslistener.h"
#endif

FileSystemWatcherInterface::FileSystemWatcherInterface(QObject* parent)
    : QObject(parent) {
}

FileSystemWatcherInterface* FileSystemWatcherInterface::Create(QObject* parent) {
  FileSystemWatcherInterface* ret;
#ifdef Q_OS_DARWIN
  ret = new MacFSListener(parent);
#else
  ret = new QtFSListener(parent);
#endif

  ret->Init();
  return ret;
}
