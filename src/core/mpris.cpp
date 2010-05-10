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

#include "mpris.h"

#include <QCoreApplication>

#include "mpris_root.h"

QDBusArgument& operator<< (QDBusArgument& arg, const Version& version) {
  arg.beginStructure();
  arg << version.major << version.minor;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg, Version& version) {
  arg.beginStructure();
  arg >> version.major >> version.minor;
  arg.endStructure();
  return arg;
}

MPRIS::MPRIS(QObject* parent)
    : QObject(parent) {
  new MprisRoot(this);
  QDBusConnection::sessionBus().registerObject("/", this);
}

QString MPRIS::Identity() {
  return QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion());
}

Version MPRIS::MprisVersion() {
  Version version;
  version.major = 1;
  version.minor = 0;
  return version;
}

void MPRIS::Quit() {
  qApp->quit();
}
