#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QThread>
#include <QtDebug>

#include <boost/shared_ptr.hpp>

class BackgroundThreadBase : public QThread {
  Q_OBJECT
 public:
  BackgroundThreadBase(QObject* parent = 0) : QThread(parent) {}
  virtual ~BackgroundThreadBase() {}
 signals:
  void Initialised();
};

template <typename T>
class BackgroundThread : public BackgroundThreadBase {
 public:
  BackgroundThread(QObject* parent = 0);
  virtual ~BackgroundThread();

  boost::shared_ptr<T> Worker() const { return worker_; }

 protected:
  void run();

 private:
  boost::shared_ptr<T> worker_;
};

template <typename T>
BackgroundThread<T>::BackgroundThread(QObject *parent)
  : BackgroundThreadBase(parent)
{
}

template <typename T>
BackgroundThread<T>::~BackgroundThread() {
  if (isRunning()) {
    quit();
    if (wait(10000))
      return;
    terminate();
    wait(10000);
  }
}

template <typename T>
void BackgroundThread<T>::run() {
  worker_.reset(new T);

  emit Initialised();
  exec();

  worker_.reset();
}

#endif // BACKGROUNDTHREAD_H
