/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef SIGNALCHECKER_H
#define SIGNALCHECKER_H

#include <glib-object.h>

#include <boost/function_types/function_arity.hpp>
#include <boost/typeof/typeof.hpp>

// Do not call this directly, use CHECKED_GCONNECT instead.
bool CheckedGConnect(gpointer source, const char* signal, GCallback callback,
                     gpointer data, const int callback_param_count);

#define FUNCTION_ARITY(callback) \
  boost::function_types::function_arity<BOOST_TYPEOF(callback)>::value

#define CHECKED_GCONNECT(source, signal, callback, data)      \
  CheckedGConnect(source, signal, G_CALLBACK(callback), data, \
                  FUNCTION_ARITY(callback));

#endif  // SIGNALCHECKER_H
