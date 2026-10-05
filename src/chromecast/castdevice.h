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

#ifndef CHROMECAST_CASTDEVICE_H_
#define CHROMECAST_CASTDEVICE_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QString>

class QDebug;

// A Cast device, or a speaker group, found on the network.
struct CastDevice {
  // From the "id" TXT entry. It stays the same when the device restarts or
  // changes address.
  QString id;
  // What the user called it ("fn"), eg. "Living Room TV".
  QString name;
  // "md", eg. "Chromecast Ultra".
  QString model;
  // Where its control connection is. Speaker groups use a port of their own
  // on one of their members.
  QHostAddress address;
  quint16 port = 0;

  bool is_valid() const {
    return !id.isEmpty() && !address.isNull() && port != 0;
  }

  // Fills in id, name and model from a _googlecast._tcp TXT record, whose
  // entries look like "key=value". Returns false if it has no id.
  bool ParseTxt(const QList<QByteArray>& txt);

  bool operator==(const CastDevice& other) const;
  bool operator!=(const CastDevice& other) const { return !(*this == other); }
};

QDebug operator<<(QDebug dbg, const CastDevice& device);

#endif  // CHROMECAST_CASTDEVICE_H_
