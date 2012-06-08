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

#include <boost/config.hpp>

#ifdef BOOST_NO_VARIADIC_TEMPLATES

template <typename R>
int GetFunctionParamCount(R (*func)()) {
  return 0;
}

template <typename R, typename P0>
int GetFunctionParamCount(R (*func)(P0)) {
  return 1;
}

template <typename R, typename P0, typename P1>
int GetFunctionParamCount(R (*func)(P0, P1)) {
  return 2;
}

template <typename R, typename P0, typename P1, typename P2>
int GetFunctionParamCount(R (*func)(P0, P1, P2)) {
  return 3;
}

#else  // BOOST_NO_VARIADIC_TEMPLATES

template <typename R, typename... Params>
int GetFunctionParamCount(R (*func)(Params...)) {
  return sizeof...(Params);
}

#endif  // BOOST_NO_VARIADIC_TEMPLATES

bool CheckedGConnect(
    gpointer source,
    const char* signal,
    GCallback callback,
    gpointer data,
    const int callback_param_count);

#ifdef BOOST_NO_VARIADIC_TEMPLATES

template <typename R, typename P0, typename P1, typename P2>
bool CheckedGConnect(
    gpointer source, const char* signal, R (*callback)(P0, P1, P2), gpointer data) {
  return CheckedGConnect(
      source, signal, G_CALLBACK(callback), data, GetFunctionParamCount(callback));
}

#else

template <typename R, typename... Params>
bool CheckedGConnect(
    gpointer source, const char* signal, R (*callback)(Params...), gpointer data) {
  return CheckedGConnect(
      source, signal, G_CALLBACK(callback), data, GetFunctionParamCount(callback));
}

#endif  // BOOST_NO_VARIADIC_TEMPLATES

#endif  // SIGNALCHECKER_H
