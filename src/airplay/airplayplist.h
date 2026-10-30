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

#ifndef AIRPLAY_AIRPLAYPLIST_H_
#define AIRPLAY_AIRPLAYPLIST_H_

#include <QByteArray>
#include <QVariant>

// Apple's binary property lists ("bplist00"), as QVariants: QVariantMap for a
// dict, QVariantList for an array, QString, QByteArray for data, bool, qint64
// for integers and double for reals (and dates, as seconds since 2001).
namespace airplay {
namespace bplist {

extern const char kContentType[];

QByteArray Write(const QVariant& root);
// False if |data| isn't a binary property list this understands.
bool Read(const QByteArray& data, QVariant* root);

}  // namespace bplist
}  // namespace airplay

#endif  // AIRPLAY_AIRPLAYPLIST_H_
