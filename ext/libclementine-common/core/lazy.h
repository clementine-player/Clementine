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
// Usage1:
//    Lazy<Foo> my_lazy_object([]() { return new Foo; });
// Usage2:
//    Lazy<Foo> my_lazy_object([]() { return new Foo; },
//                             [](Foo *foo) { delete foo; });
// Note: the Lazy::default_deleter just deletes the object. The default
//       unique_ptr would have a specialization for arrays. The second usage
//       should be used with arrays.
//
template <typename T>
class Lazy {
 public:
  explicit Lazy(std::function<T*()> init,
                std::function<void(T*)> deleter = Lazy::default_deleter)
      : init_(init), ptr_(nullptr, deleter) {}

  // Convenience constructor that will lazily default construct the object.
  Lazy()
      : init_([]() { return new T; }), ptr_(nullptr, Lazy::default_deleter) {}

  T* get() const {
    CheckInitialised();
    return ptr_.get();
  }

  typename std::add_lvalue_reference<T>::type operator*() const {
    CheckInitialised();
    return *ptr_;
  }

  T* operator->() const { return get(); }

  // Returns true if the object is not yet initialised.
  explicit operator bool() const { return ptr_; }

  // Deletes the underlying object and will re-run the initialisation function
  // if the object is requested again.
  void reset() { ptr_.reset(nullptr); }

  static void default_deleter(T* obj) { delete obj; }

 private:
  void CheckInitialised() const {
    if (!ptr_) {
      ptr_.reset(init_());
    }
  }

  const std::function<T*()> init_;
  mutable std::unique_ptr<T, std::function<void(T*)>> ptr_;
};

#endif  // LAZY_H
