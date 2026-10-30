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

#ifndef AIRPLAY_AIRPLAYDEVICE_H_
#define AIRPLAY_AIRPLAYDEVICE_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QString>

class QDebug;

// An AirPlay receiver found on the network, from its _airplay._tcp
// advertisement.
struct AirPlayDevice {
  // Bits of the status flags ("flags" in the TXT record, and statusFlags in
  // GET /info). A Mac (AirTunes 980.77.5) was seen to set 0x200 while its
  // "Require password" is on, and to clear it when that's off.
  enum StatusFlag {
    PasswordRequired = 0x200,
  };

  // From the "deviceid" TXT entry, which looks like a MAC address. It stays
  // the same when the receiver restarts or changes address.
  QString id;
  // The service's instance name, which is what the user called it, eg.
  // "Living Room".
  QString name;
  // "model", eg. "Mac14,2" or "AudioAccessory5,1".
  QString model;
  // "features" (or "ft"), two 32-bit hex words, low then high.
  quint64 features = 0;
  // "flags" (or "sf"), a bitmask of StatusFlag and others.
  quint32 status_flags = 0;
  // Where its control connection is.
  QHostAddress address;
  quint16 port = 0;

  bool password_required() const { return status_flags & PasswordRequired; }
  // Whether it does HomeKit pairing, which AirPlay 2 needs, as pyatv
  // decides.
  bool supports_airplay2() const;

  bool is_valid() const {
    return !id.isEmpty() && !address.isNull() && port != 0;
  }

  // Fills in id, model, features and status flags from a _airplay._tcp TXT
  // record, whose entries look like "key=value". Returns false if it has no
  // device id.
  bool ParseTxt(const QList<QByteArray>& txt);

  // Splits a TXT record in its wire format, where each entry is preceded by
  // its length in one byte, into the entries ParseTxt takes. A truncated last
  // entry is dropped.
  static QList<QByteArray> SplitTxt(const QByteArray& rdata);

  bool operator==(const AirPlayDevice& other) const;
  bool operator!=(const AirPlayDevice& other) const {
    return !(*this == other);
  }
};

QDebug operator<<(QDebug dbg, const AirPlayDevice& device);

#endif  // AIRPLAY_AIRPLAYDEVICE_H_
