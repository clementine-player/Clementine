#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QThread>
#include <QtDebug>

#include <boost/shared_ptr.hpp>

#ifdef Q_OS_LINUX
#  include <sys/syscall.h>
#endif
#ifdef Q_OS_DARWIN
#  include <sys/resource.h>
#endif

class BackgroundThreadBase : public QThread {
  Q_OBJECT
 public:
  BackgroundThreadBase(QObject* parent = 0) : QThread(parent), io_priority_(IOPRIO_CLASS_NONE) {}
  virtual ~BackgroundThreadBase() {}

  // Borrowed from schedutils
  enum IoPriority {
    IOPRIO_CLASS_NONE = 0,
    IOPRIO_CLASS_RT,
    IOPRIO_CLASS_BE,
    IOPRIO_CLASS_IDLE,
  };

  void set_io_priority(IoPriority priority) { io_priority_ = priority; }

 signals:
  void Initialised();

 protected:
  // Borrowed from schedutils
  static inline int ioprio_set(int which, int who, int ioprio);
  static inline int gettid();

  enum {
    IOPRIO_WHO_PROCESS = 1,
    IOPRIO_WHO_PGRP,
    IOPRIO_WHO_USER,
  };
  static const int IOPRIO_CLASS_SHIFT = 13;

  IoPriority io_priority_;
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
#ifdef Q_OS_LINUX
  if (io_priority_ != IOPRIO_CLASS_NONE) {
    ioprio_set(IOPRIO_WHO_PROCESS, gettid(),
               4 | io_priority_ << IOPRIO_CLASS_SHIFT);
  }
#endif

  worker_.reset(new T);

  emit Initialised();
  exec();

  worker_.reset();
}

int BackgroundThreadBase::ioprio_set(int which, int who, int ioprio) {
#ifdef Q_OS_LINUX
  return syscall(SYS_ioprio_set, which, who, ioprio);
#elif defined(Q_OS_DARWIN)
  return setpriority(PRIO_DARWIN_THREAD, 0, ioprio == IOPRIO_CLASS_IDLE ? PRIO_DARWIN_BG : 0);
#else
  return 0;
#endif
}

int BackgroundThreadBase::gettid() {
#ifdef Q_OS_LINUX
  return syscall(SYS_gettid);
#else
  return 0;
#endif
}

#endif // BACKGROUNDTHREAD_H
