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

#include "airplaydmap.h"

#include <QtEndian>

#include "airplaycrypto.h"

namespace airplay {
namespace dmap {

QByteArray Write(const Items& items) {
  QByteArray out;
  for (const auto& item : items) {
    out += item.first.left(4);
    out += Bytes(qToBigEndian(quint32(item.second.size())));
    out += item.second;
  }
  return out;
}

Items Read(const QByteArray& data) {
  Items items;
  int pos = 0;
  while (pos + 8 <= data.size()) {
    const quint32 length = qFromBigEndian<quint32>(data.constData() + pos + 4);
    if (length > quint32(data.size() - pos - 8)) break;
    items.append(qMakePair(data.mid(pos, 4), data.mid(pos + 8, length)));
    pos += 8 + length;
  }
  return items;
}

QByteArray Get(const Items& items, const QByteArray& tag) {
  for (const auto& item : items) {
    if (item.first == tag) return item.second;
  }
  return QByteArray();
}

}  // namespace dmap
}  // namespace airplay
