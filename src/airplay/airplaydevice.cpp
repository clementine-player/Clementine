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

#include "airplaydevice.h"

#include <QDebug>
#include <QStringList>

namespace {

// Features that mean HomeKit pairing: SupportsHKPairingAndAccessControl and
// SupportsUnifiedPairSetupAndMFi, as named by openairplay.github.io and
// pyatv.
const quint64 kHomeKitPairing = quint64(1) << 38;
const quint64 kUnifiedPairSetup = quint64(1) << 48;

quint32 ParseHex(const QString& value) {
  QString digits = value.trimmed();
  if (digits.startsWith("0x", Qt::CaseInsensitive)) digits = digits.mid(2);
  return digits.toUInt(nullptr, 16);
}

}  // namespace

bool AirPlayDevice::supports_airplay2() const {
  return features & (kHomeKitPairing | kUnifiedPairSetup);
}

bool AirPlayDevice::ParseTxt(const QList<QByteArray>& txt) {
  QString new_id;
  QString new_model;
  QString new_features;
  QString new_flags;
  for (const QByteArray& entry : txt) {
    const int equals = entry.indexOf('=');
    if (equals <= 0) continue;
    const QByteArray key = entry.left(equals);
    const QString value = QString::fromUtf8(entry.mid(equals + 1));
    if (key == "deviceid") {
      new_id = value;
    } else if (key == "model") {
      new_model = value;
    } else if (key == "features" || (key == "ft" && new_features.isEmpty())) {
      new_features = value;
    } else if (key == "flags" || (key == "sf" && new_flags.isEmpty())) {
      new_flags = value;
    }
  }
  if (new_id.isEmpty()) return false;

  id = new_id;
  model = new_model;
  const QStringList words = new_features.split(',');
  features = quint64(ParseHex(words.value(0))) |
             (quint64(ParseHex(words.value(1))) << 32);
  status_flags = ParseHex(new_flags);
  return true;
}

bool AirPlayDevice::ParseRaopService(const QString& instance,
                                     const QList<QByteArray>& txt) {
  const int at = instance.indexOf('@');
  const QString mac = instance.left(at);
  if (at != 12 || QByteArray::fromHex(mac.toLatin1()).size() != 6) {
    return false;
  }
  QStringList octets;
  for (int i = 0; i < 12; i += 2) octets << mac.mid(i, 2).toUpper();

  QString new_model;
  QString new_features;
  QString new_flags;
  bool password = false;
  bool plain = true;
  for (const QByteArray& entry : txt) {
    const int equals = entry.indexOf('=');
    if (equals <= 0) continue;
    const QByteArray key = entry.left(equals);
    const QString value = QString::fromUtf8(entry.mid(equals + 1));
    if (key == "am") {
      new_model = value;
    } else if (key == "ft") {
      new_features = value;
    } else if (key == "sf") {
      new_flags = value;
    } else if (key == "pw") {
      password = value.compare("true", Qt::CaseInsensitive) == 0;
    } else if (key == "et") {
      plain = value.split(',').contains("0");
    }
  }

  id = octets.join(':');
  name = instance.mid(at + 1);
  model = new_model;
  const QStringList words = new_features.split(',');
  features = quint64(ParseHex(words.value(0))) |
             (quint64(ParseHex(words.value(1))) << 32);
  status_flags = ParseHex(new_flags);
  if (password) status_flags |= PasswordRequired;
  plain_audio = plain;
  airplay1 = true;
  return true;
}

QList<QByteArray> AirPlayDevice::SplitTxt(const QByteArray& rdata) {
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

bool AirPlayDevice::operator==(const AirPlayDevice& other) const {
  return id == other.id && name == other.name && model == other.model &&
         features == other.features && status_flags == other.status_flags &&
         airplay1 == other.airplay1 && plain_audio == other.plain_audio &&
         address == other.address && port == other.port;
}

QDebug operator<<(QDebug dbg, const AirPlayDevice& device) {
  QDebugStateSaver saver(dbg);
  dbg.nospace().noquote() << device.name << " (" << device.model << ", "
                          << device.id << ") at " << device.address.toString()
                          << ":" << device.port;
  if (device.password_required()) dbg << ", password required";
  return dbg;
}
