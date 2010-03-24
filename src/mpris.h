/* This file is part of Clementine.

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

#ifndef MPRIS_H
#define MPRIS_H

#include <QDBusArgument>
#include <QObject>

struct Version {
  quint16 minor;
  quint16 major;
};
Q_DECLARE_METATYPE(Version)

QDBusArgument& operator<< (QDBusArgument& arg, const Version& version);
const QDBusArgument& operator>> (const QDBusArgument& arg, Version& version);

class MPRIS : public QObject {
  Q_OBJECT
 public:
  MPRIS(QObject* parent = 0);
  QString Identity();
  void Quit();
  Version MprisVersion();
};

#endif // MPRIS_H
