/* This file is part of Clementine.
   Copyright 2016, John Maguire <john.maguire@gmail.com>

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

#ifndef LAZY_H
#define LAZY_H

#include <functional>
#include <memory>

// Helper for lazy initialisation of objects.
// Usage:
//    Lazy<Foo> my_lazy_object([]() { return new Foo; });
template <typename T>
class Lazy {
 public:
  explicit Lazy(std::function<T*()> init) : init_(init) {}

  T* get() const {
    CheckInitialised();
    return ptr_.get();
  }

  typename std::add_lvalue_reference<T>::type operator*() const {
    CheckInitialised();
    return *ptr_;
  }

  T* operator->() const { return get(); }

 private:
  void CheckInitialised() const {
    if (!ptr_) {
      ptr_.reset(init_());
    }
  }

  const std::function<T*()> init_;
  mutable std::unique_ptr<T> ptr_;
};

#endif  // LAZY_H
