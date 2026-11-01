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

// An AirPlay receiver found on the network, from its _airplay._tcp or
// _raop._tcp advertisement.
struct AirPlayDevice {
  // Bits of the status flags ("flags" in the TXT record, and statusFlags in
  // GET /info). A Mac (AirTunes 980.77.5) was seen to set 0x200 while its
  // "Require password" is on, and to clear it when that's off.
  enum StatusFlag {
    // Needs a PIN that it shows (bit 3), as openairplay.github.io lists it.
    // Not yet seen on a receiver.
    PinRequired = 0x8,
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
  // Whether it's from a _raop._tcp advertisement, and so is played to with
  // AirPlay 1.
  bool airplay1 = false;
  // Whether it takes unencrypted AirPlay 1 audio: "et" (encryption types)
  // includes 0, or is missing. AirPort Express-era receivers want audio
  // encrypted with a key wrapped in Apple's RSA key, which isn't supported.
  bool plain_audio = true;
  // Where its control connection is.
  QHostAddress address;
  quint16 port = 0;
  // For an AirPlay 2 receiver that also advertises _raop._tcp: where that
  // is, and whether it takes unencrypted audio, to play with AirPlay 1 if
  // it refuses an AirPlay 2 session, as shairport-sync's AirPlay 2 builds
  // do with NTP timing. raop_port is 0 if it doesn't.
  QHostAddress raop_address;
  quint16 raop_port = 0;
  bool raop_plain_audio = true;

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

  // Fills in id, name, model, features and status flags from a _raop._tcp
  // advertisement, such as an AirPort Express's:
  //
  //   instance  A1B2C3D4E5F6@Living Room
  //   TXT       txtvers=1 ch=2 cn=0,1 et=0,3,5 sv=false da=true sr=44100
  //             ss=16 pw=false sf=0x4 ft=0x445F8A00,0x1C340 tp=UDP
  //             vn=65537 vs=366.0 am=AirPort10,115
  //
  // The instance name is the MAC address, which becomes the id
  // ("A1:B2:C3:D4:E5:F6"), then "@" and the name. Of the TXT record, only
  // "am" (the model), "ft" and "sf" (features and status flags), "pw"
  // (whether there's a password) and "et" (encryption types, where 0 is
  // unencrypted) are used. Returns false if the instance name has no MAC.
  bool ParseRaopService(const QString& instance, const QList<QByteArray>& txt);

  // Splits a TXT record in its wire format, where each entry is preceded by
  // its length in one byte, into the entries ParseTxt takes. A truncated last
  // entry is dropped.
  static QList<QByteArray> SplitTxt(const QByteArray& rdata);

  bool operator==(const AirPlayDevice& other) const = default;
};

QDebug operator<<(QDebug dbg, const AirPlayDevice& device);

#endif  // AIRPLAY_AIRPLAYDEVICE_H_
