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

#ifndef UPNP_SONOSTOPOLOGY_H_
#define UPNP_SONOSTOPOLOGY_H_

#include <QList>
#include <QString>

// One group of rooms from a Sonos player's ZoneGroupTopology service, which
// is how UpnpDiscovery tells which Sonos MediaRenderers can be played on.
//
// A group plays what its coordinator plays, so only the coordinator's
// AVTransport is any use; telling another member to play takes it out of its
// group.
struct SonosZoneGroup {
  struct Member {
    // The player's root device UDN, without "uuid:".
    QString id;
    QString room;
    // The second speaker of a stereo pair, which has no room of its own.
    bool invisible = false;
  };

  QString coordinator_id;
  QList<Member> members;

  // The rooms, coordinator first, as the Sonos app names a group: "Lounge"
  // or "Lounge + Kitchen".
  QString Name() const;

  // Parses GetZoneGroupState's ZoneGroupState, with or without the
  // <ZoneGroupState> around the <ZoneGroups> that older firmware leaves out.
  static QList<SonosZoneGroup> ParseState(const QString& xml);
};

#endif  // UPNP_SONOSTOPOLOGY_H_
