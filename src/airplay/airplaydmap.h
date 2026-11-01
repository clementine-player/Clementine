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

#ifndef AIRPLAY_AIRPLAYDMAP_H_
#define AIRPLAY_AIRPLAYDMAP_H_

#include <QByteArray>
#include <QList>
#include <QPair>

// DMAP, the tagged format of iTunes' DAAP, which AirPlay senders send the
// now-playing song in: each item is a four-letter tag, its length as 32 bits
// big endian, and its value. A container's value is more items.
namespace airplay {
namespace dmap {

using Items = QList<QPair<QByteArray, QByteArray>>;

QByteArray Write(const Items& items);
// One level of items: a container's value can be read in turn.
Items Read(const QByteArray& data);
// The first item tagged |tag|, or a null QByteArray.
QByteArray Get(const Items& items, const QByteArray& tag);

}  // namespace dmap
}  // namespace airplay

#endif  // AIRPLAY_AIRPLAYDMAP_H_
