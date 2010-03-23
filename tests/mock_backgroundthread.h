#ifndef MOCK_BACKGROUNDTHREAD_H
#define MOCK_BACKGROUNDTHREAD_H

#include "backgroundthread.h"

template <typename InterfaceType, typename DerivedType>
class FakeBackgroundThread : public BackgroundThread<InterfaceType> {
 public:
  FakeBackgroundThread(QObject* parent) : BackgroundThread<InterfaceType>(parent) {
    // We have to create the actual worker here instead of in Start() so that
    // tests can set mock expectations on it before Initialised is emitted.
    BackgroundThread<InterfaceType>::worker_.reset(new DerivedType);
  }

  void Start() {
    emit BackgroundThreadBase::Initialised();
  }
};

template <typename InterfaceType, typename DerivedType>
class FakeBackgroundThreadFactory : public BackgroundThreadFactory<InterfaceType> {
 public:
  BackgroundThread<InterfaceType>* GetThread(QObject* parent) {
    return new FakeBackgroundThread<InterfaceType, DerivedType>(parent);
  }
};

#endif
