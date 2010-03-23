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

// These classes are a bit confusing because they're trying to do so many
// things:
//  * Run a worker in a background thread
//  * ... or maybe run it in the same thread if we're in a test
//  * Use interfaces throughout, so the implementations can be mocked
//  * Create concrete implementations of the interfaces when threads start
//
// The types you should use throughout your header files are:
//   BackgroundThread<InterfaceType>
//   BackgroundThreadFactory<InterfaceType>
//
// You should allow callers to set their own factory (which might return mocks
// of your interface), and default to using a:
//   BackgroundThreadFactoryImplementation<InterfaceType, DerivedType>


// This is the base class.  We need one because moc doesn't like templated
// classes.  This also deals with anything that doesn't depend on the type of
// the worker.
class BackgroundThreadBase : public QThread {
  Q_OBJECT
 public:
  BackgroundThreadBase(QObject* parent = 0) : QThread(parent), io_priority_(IOPRIO_CLASS_NONE) {}

  // Borrowed from schedutils
  enum IoPriority {
    IOPRIO_CLASS_NONE = 0,
    IOPRIO_CLASS_RT,
    IOPRIO_CLASS_BE,
    IOPRIO_CLASS_IDLE,
  };

  void set_io_priority(IoPriority priority) { io_priority_ = priority; }
  void set_cpu_priority(QThread::Priority priority) { cpu_priority_ = priority; }

  virtual void Start() { QThread::start(cpu_priority_); }

 signals:
  void Initialised();

 protected:
  int SetIOPriority();
  static int gettid();

  enum {
    IOPRIO_WHO_PROCESS = 1,
    IOPRIO_WHO_PGRP,
    IOPRIO_WHO_USER,
  };
  static const int IOPRIO_CLASS_SHIFT = 13;

  IoPriority io_priority_;
  QThread::Priority cpu_priority_;
};

// This is the templated class that stores and returns the worker object.
template <typename InterfaceType>
class BackgroundThread : public BackgroundThreadBase {
 public:
  BackgroundThread(QObject* parent = 0);
  ~BackgroundThread();

  boost::shared_ptr<InterfaceType> Worker() const { return worker_; }

 protected:
  boost::shared_ptr<InterfaceType> worker_;
};

// This class actually creates an implementation of the worker object
template <typename InterfaceType, typename DerivedType>
class BackgroundThreadImplementation : public BackgroundThread<InterfaceType> {
 public:
  BackgroundThreadImplementation(QObject* parent = 0);

 protected:
  void run();
};


// This is a pure virtual factory for creating threads.
template <typename InterfaceType>
class BackgroundThreadFactory {
 public:
  virtual ~BackgroundThreadFactory() {}
  virtual BackgroundThread<InterfaceType>* GetThread(QObject* parent) = 0;
};

// This implementation of the factory returns a BackgroundThread that creates
// the right derived types...
template <typename InterfaceType, typename DerivedType>
class BackgroundThreadFactoryImplementation : public BackgroundThreadFactory<InterfaceType> {
 public:
  BackgroundThread<InterfaceType>* GetThread(QObject* parent) {
    return new BackgroundThreadImplementation<InterfaceType, DerivedType>(parent);
  }
};



template <typename InterfaceType>
BackgroundThread<InterfaceType>::BackgroundThread(QObject *parent)
  : BackgroundThreadBase(parent)
{
}

template <typename InterfaceType>
BackgroundThread<InterfaceType>::~BackgroundThread() {
  if (isRunning()) {
    quit();
    if (wait(10000))
      return;
    terminate();
    wait(10000);
  }
}

template <typename InterfaceType, typename DerivedType>
BackgroundThreadImplementation<InterfaceType, DerivedType>::
    BackgroundThreadImplementation(QObject* parent)
      : BackgroundThread<InterfaceType>(parent)
{
}


template <typename InterfaceType, typename DerivedType>
void BackgroundThreadImplementation<InterfaceType, DerivedType>::run() {
  BackgroundThreadBase::SetIOPriority();

  BackgroundThread<InterfaceType>::worker_.reset(new DerivedType);

  emit BackgroundThreadBase::Initialised();
  QThread::exec();

  BackgroundThread<InterfaceType>::worker_.reset();
}




#endif // BACKGROUNDTHREAD_H
