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

#include "castdevice.h"

#include <QDebug>

bool CastDevice::ParseTxt(const QList<QByteArray>& txt) {
  QString new_id;
  QString new_name;
  QString new_model;
  for (const QByteArray& entry : txt) {
    const int equals = entry.indexOf('=');
    if (equals <= 0) continue;
    const QByteArray key = entry.left(equals);
    const QString value = QString::fromUtf8(entry.mid(equals + 1));
    if (key == "id") {
      new_id = value;
    } else if (key == "fn") {
      new_name = value;
    } else if (key == "md") {
      new_model = value;
    }
  }
  if (new_id.isEmpty()) return false;

  id = new_id;
  name = new_name.isEmpty() ? new_model : new_name;
  model = new_model;
  return true;
}

QList<QByteArray> CastDevice::SplitTxt(const QByteArray& rdata) {
  QList<QByteArray> entries;
  int pos = 0;
  while (pos < rdata.size()) {
    const int length = static_cast<quint8>(rdata[pos++]);
    if (pos + length > rdata.size()) break;
    if (length > 0) entries << rdata.mid(pos, length);
    pos += length;
  }
  return entries;
}

bool CastDevice::operator==(const CastDevice& other) const {
  return id == other.id && name == other.name && model == other.model &&
         address == other.address && port == other.port;
}

QDebug operator<<(QDebug dbg, const CastDevice& device) {
  QDebugStateSaver saver(dbg);
  dbg.nospace().noquote() << device.name << " (" << device.model << ", "
                          << device.id << ") at " << device.address.toString()
                          << ":" << device.port;
  return dbg;
}
