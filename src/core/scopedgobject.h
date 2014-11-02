/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef SCOPEDGOBJECT_H
#define SCOPEDGOBJECT_H

#include <glib-object.h>

#include <QtDebug>

template <typename T>
class ScopedGObject {
 public:
  ScopedGObject() : object_(nullptr) {}

  explicit ScopedGObject(const ScopedGObject& other) : object_(nullptr) {
    reset(other.object_);
  }

  ~ScopedGObject() { reset(); }

  ScopedGObject& operator=(const ScopedGObject& other) {
    reset(other.object_);
    return *this;
  }

  void reset(T* new_object = nullptr) {
    if (new_object) g_object_ref(new_object);
    reset_without_add(new_object);
  }

  void reset_without_add(T* new_object = nullptr) {
    if (object_) g_object_unref(object_);

    object_ = new_object;
  }

  T* get() const { return object_; }
  operator T*() const { return get(); }
  T* operator*() const { return get(); }
  operator bool() const { return get(); }

  bool operator==(const ScopedGObject& other) const {
    return object_ == other.object_;
  }

 private:
  T* object_;
};

#endif  // SCOPEDGOBJECT_H
