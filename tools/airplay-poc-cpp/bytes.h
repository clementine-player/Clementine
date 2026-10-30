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

#ifndef AIRPLAY_BYTES_H_
#define AIRPLAY_BYTES_H_

#include <QByteArray>
#include <concepts>

namespace airplay {

// The bytes of |value| as they are in memory, so convert it first:
// Bytes(qToBigEndian(seq)).
template <std::integral T>
QByteArray Bytes(T value) {
  return QByteArray(reinterpret_cast<const char*>(&value), sizeof(value));
}

}  // namespace airplay

#endif  // AIRPLAY_BYTES_H_
