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

#include "airplaytlv8.h"

namespace airplay {
namespace tlv8 {

QByteArray Write(const Items& items) {
  QByteArray out;
  for (const auto& item : items) {
    const QByteArray& value = item.second;
    if (value.isEmpty()) {
      out.append(char(item.first));
      out.append('\0');
    }
    for (int pos = 0; pos < value.size(); pos += 255) {
      const QByteArray chunk = value.mid(pos, 255);
      out.append(char(item.first));
      out.append(char(chunk.size()));
      out.append(chunk);
    }
  }
  return out;
}

Items Read(const QByteArray& data) {
  Items items;
  int pos = 0;
  while (pos + 2 <= data.size()) {
    const quint8 type = quint8(data[pos]);
    const int length = quint8(data[pos + 1]);
    const QByteArray value = data.mid(pos + 2, length);
    // A value continues in the next item when it has the same type.
    if (!items.isEmpty() && items.last().first == type) {
      items.last().second += value;
    } else {
      items.append(qMakePair(type, value));
    }
    pos += 2 + length;
  }
  return items;
}

QByteArray Get(const Items& items, quint8 type) {
  for (const auto& item : items) {
    if (item.first == type) return item.second;
  }
  return QByteArray();
}

}  // namespace tlv8
}  // namespace airplay
