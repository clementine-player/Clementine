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
#include "serverid.h"

#include <QSettings>
#include <QUuid>

#include "networkremote/networkremote.h"

QString NetworkRemoteServerId() {
  QSettings s;
  s.beginGroup(NetworkRemote::kSettingsGroup);
  QString id = s.value("server_id").toString();
  if (id.isEmpty()) {
    id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.setValue("server_id", id);
  }
  return id;
}
