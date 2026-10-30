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

#ifndef AIRPLAY_DISCOVERY_H_
#define AIRPLAY_DISCOVERY_H_

#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class QDBusMessage;

namespace airplay {

enum class Version { kAirPlay1, kAirPlay2 };
QString VersionName(Version version);

// The bit of a receiver's status flags (flags and sf in its TXT records,
// statusFlags in GET /info) that a Mac sets while "Require password" is on.
const quint32 kStatusPasswordRequired = 0x200;

struct Device {
  QString name;
  QList<QHostAddress> addresses;
  // 0 if the receiver doesn't advertise the service.
  quint16 raop_port = 0;
  quint16 airplay_port = 0;
  QMap<QString, QString> txt;

  quint64 features() const;
  // AirPlay 2 if it does HomeKit-style pairing, as pyatv decides.
  Version version() const;
  // From the status flags, or pw=true, which older AirPlay 1 receivers use.
  bool password_required() const;
  quint16 port() const { return raop_port ? raop_port : airplay_port; }
  // The first IPv4 address, else the first address.
  QHostAddress address() const;
};

// Looks for AirPlay receivers (_raop._tcp and _airplay._tcp) with avahi-daemon
// over D-Bus, for a while, then reports what it found.
class Discovery : public QObject {
  Q_OBJECT

 public:
  explicit Discovery(QObject* parent = nullptr);

  void Browse(int msec);

 signals:
  // Sorted by name.
  void Finished(const QList<airplay::Device>& devices);

 private slots:
  void ItemNew(int iface, int protocol, const QString& name,
               const QString& type, const QString& domain, uint flags);
  void Done();

 private:
  void BrowserPrepared(const QString& type, const QDBusMessage& reply);
  void Resolved(const QDBusMessage& reply);

  QMap<QString, Device> devices_;
  QStringList browsers_;
};

}  // namespace airplay

#endif  // AIRPLAY_DISCOVERY_H_
