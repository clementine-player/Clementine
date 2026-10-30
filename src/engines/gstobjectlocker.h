/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef ENGINES_GSTOBJECTLOCKER_H_
#define ENGINES_GSTOBJECTLOCKER_H_

#include <gst/gst.h>

// Holds a GstObject's lock for as long as it's in scope, as QMutexLocker
// does a QMutex: GST_OBJECT_LOCK when it's made, GST_OBJECT_UNLOCK when it
// goes. |object| is any GstObject, such as an element.
class GstObjectLocker {
 public:
  explicit GstObjectLocker(gpointer object) : object_(GST_OBJECT(object)) {
    GST_OBJECT_LOCK(object_);
  }
  ~GstObjectLocker() { GST_OBJECT_UNLOCK(object_); }

  GstObjectLocker(const GstObjectLocker&) = delete;
  GstObjectLocker& operator=(const GstObjectLocker&) = delete;

 private:
  GstObject* const object_;
};

#endif  // ENGINES_GSTOBJECTLOCKER_H_
