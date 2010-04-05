/* This file is part of Clementine.

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

#ifndef VLCSCOPEDREF_H
#define VLCSCOPEDREF_H

#include <vlc/vlc.h>

template <typename T>
class VlcScopedRef {
 public:
  VlcScopedRef(T* ptr);
  ~VlcScopedRef();

  operator T* () const { return ptr_; }
  operator bool () const { return ptr_; }
  T* operator ->() const { return ptr_; }

 private:
  VlcScopedRef(VlcScopedRef&) {}
  VlcScopedRef& operator =(const VlcScopedRef&) { return *this; }

  T* ptr_;
};


#define VLCSCOPEDREF_DEFINE2(type, dtor) \
    template <> void VlcScopedRef_Release<libvlc_##type##_t>(libvlc_##type##_t* ptr) { \
      dtor(ptr); \
    }
#define VLCSCOPEDREF_DEFINE(type) VLCSCOPEDREF_DEFINE2(type, libvlc_##type##_release)

template <typename T>
void VlcScopedRef_Release(T* ptr);

VLCSCOPEDREF_DEFINE2(instance, libvlc_release);
VLCSCOPEDREF_DEFINE(media_player);
VLCSCOPEDREF_DEFINE(media);

template <> void VlcScopedRef_Release<char>(char* ptr) { free(ptr); }

template <typename T>
VlcScopedRef<T>::VlcScopedRef(T* ptr)
  : ptr_(ptr) {
}

template <typename T>
VlcScopedRef<T>::~VlcScopedRef() {
  VlcScopedRef_Release(ptr_);
}

#endif // VLCSCOPEDREF_H
