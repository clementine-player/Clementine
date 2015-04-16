/* This file is part of Clementine.
   Copyright 2015, David Sansome <me@davidsansome.com>

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

#ifndef CORE_THREAD_H_
#define CORE_THREAD_H_

#include <QThread>
#include "core/utilities.h"

// Improve QThread by adding a SetIoPriority function
class Thread : public QThread {
 public:
  Thread(QObject* parent = nullptr)
      : QThread(parent), io_priority_(Utilities::IOPRIO_CLASS_NONE) {}

  void SetIoPriority(Utilities::IoPriority priority) {
    io_priority_ = priority;
  }
  virtual void run() override;

 private:
  Utilities::IoPriority io_priority_;
};

#endif  // CORE_THREAD_H_
