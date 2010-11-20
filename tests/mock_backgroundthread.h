/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

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
