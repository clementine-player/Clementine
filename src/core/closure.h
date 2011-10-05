/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef CLOSURE_H
#define CLOSURE_H

#include <QMetaMethod>
#include <QObject>

#include <boost/scoped_ptr.hpp>

#include "core/logging.h"

class ClosureArgumentWrapper {
 public:
  virtual ~ClosureArgumentWrapper() {}

  virtual QGenericArgument arg() const = 0;
};

template<typename T>
class ClosureArgument : public ClosureArgumentWrapper {
 public:
  ClosureArgument(const T& data) : data_(data) {}

  virtual QGenericArgument arg() const {
    return Q_ARG(T, data_);
  }

 private:
  T data_;
};

class Closure : public QObject {
  Q_OBJECT

 public:
  Closure(QObject* sender, const char* signal,
          QObject* receiver, const char* slot,
          const ClosureArgumentWrapper* val0 = 0,
          const ClosureArgumentWrapper* val1 = 0);

 private slots:
  void Invoked();
  void Cleanup();

 private:
  QMetaMethod slot_;

  boost::scoped_ptr<const ClosureArgumentWrapper> val0_;
  boost::scoped_ptr<const ClosureArgumentWrapper> val1_;
};

#define C_ARG(type, data) new ClosureArgument<type>(data)

#endif  // CLOSURE_H
