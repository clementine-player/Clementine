#ifndef FILESYSTEMWATCHERINTERFACE_H
#define FILESYSTEMWATCHERINTERFACE_H

#include <QObject>

class FileSystemWatcherInterface : public QObject {
  Q_OBJECT
 public:
  FileSystemWatcherInterface(QObject* parent = 0);
  virtual void Init() {}
  virtual void AddPath(const QString& path) = 0;
  virtual void Clear() = 0;

  static FileSystemWatcherInterface* Create(QObject* parent = 0);

 signals:
  void PathChanged(const QString& path);
};

#endif
