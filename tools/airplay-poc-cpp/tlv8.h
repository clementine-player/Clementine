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

#ifndef AIRPLAY_TLV8_H_
#define AIRPLAY_TLV8_H_

#include <QByteArray>
#include <QList>
#include <QPair>

// TLV8, the HomeKit pairing message format. Values over 255 bytes are split
// into consecutive items of the same type.
namespace airplay {
namespace tlv8 {

enum Type : quint8 {
  kMethod = 0x00,
  kSalt = 0x02,
  kPublicKey = 0x03,
  kProof = 0x04,
  kSeqNo = 0x06,
  kError = 0x07,
  kRetryDelay = 0x08,
  kFlags = 0x13,
};

const quint8 kFlagTransient = 0x10;

using Items = QList<QPair<quint8, QByteArray>>;

QByteArray Write(const Items& items);
Items Read(const QByteArray& data);
// The first item of |type|, or a null QByteArray.
QByteArray Get(const Items& items, quint8 type);

}  // namespace tlv8
}  // namespace airplay

#endif  // AIRPLAY_TLV8_H_
