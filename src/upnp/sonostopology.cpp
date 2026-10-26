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

#include "sonostopology.h"

#include <QStringList>
#include <QXmlStreamReader>

QStringList SonosZoneGroup::Rooms() const {
  QStringList rooms;
  for (const Member& member : members) {
    if (member.invisible || rooms.contains(member.room)) continue;
    if (member.id == coordinator_id) {
      rooms.prepend(member.room);
    } else {
      rooms << member.room;
    }
  }
  return rooms;
}

QList<SonosZoneGroup> SonosZoneGroup::ParseState(const QString& xml) {
  QList<SonosZoneGroup> ret;
  QXmlStreamReader reader(xml);
  while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::Invalid) {
    if (!reader.isStartElement()) continue;

    if (reader.name() == QLatin1String("ZoneGroup")) {
      SonosZoneGroup group;
      group.coordinator_id =
          reader.attributes().value("Coordinator").toString();
      ret << group;
    } else if (reader.name() == QLatin1String("ZoneGroupMember") &&
               !ret.isEmpty()) {
      // Satellites, such as a Sub, are inside their room's member and play
      // with it, so they aren't members of their own.
      const QXmlStreamAttributes attrs = reader.attributes();
      Member member;
      member.id = attrs.value("UUID").toString();
      member.room = attrs.value("ZoneName").toString();
      member.invisible = attrs.value("Invisible") == QLatin1String("1");
      ret.last().members << member;
      reader.skipCurrentElement();
    }
  }
  return ret;
}
