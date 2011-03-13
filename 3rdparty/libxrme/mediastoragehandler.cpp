/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "connection.h"
#include "extensions.h"
#include "mediastoragehandler.h"
#include "mediastorageinterface.h"

#include <QBuffer>
#include <QtDebug>

#include <gloox/client.h>
#include <gloox/disco.h>

namespace xrme {

MediaStorageHandler::MediaStorageHandler(MediaStorageInterface* iface)
    : interface_(iface) {
  interface_->Attach(this);
}

void MediaStorageHandler::GetArtists() {
  if (!connection_) {
    return;
  }

  foreach (const Connection::Peer& peer, connection_->peers(Connection::Peer::RemoteControl)) {
    gloox::JID to(client_->jid().bareJID());
    to.setResource(peer.jid_resource_.toUtf8().constData());

    gloox::IQ iq(gloox::IQ::Set, to, client_->getID());
    gloox::Tag* tag = iq.tag();
    gloox::Tag* c = new gloox::Tag(tag, "xrme");
    c->setXmlns(kXmlnsXrmeRemoteControl);

    gloox::Tag* artists = new gloox::Tag(c, "artists");
    gloox::Tag* artist = new gloox::Tag(artists, "artist");
    artist->addAttribute("name", "Muse");
    artist->addAttribute("songs", 42);
    artist->addAttribute("albums", 6);

    gloox::Tag* artist2 = new gloox::Tag(artists, "artist");
    artist2->addAttribute("name", "Rage against the Machine");
    artist2->addAttribute("songs", 67);
    artist2->addAttribute("albums", 8);

    client_->send(tag);
  }
}

void MediaStorageHandler::Init(Connection* connection, gloox::Client* client) {
  Handler::Init(connection, client);

  qDebug() << "Registering for extension:" << MediaStorageExtension::kExtensionType;
  client->registerIqHandler(
      this, MediaStorageExtension::kExtensionType);
  client->disco()->addFeature(kXmlnsXrmeMediaStorage);
}

bool MediaStorageHandler::handleIq(const gloox::IQ& stanza) {
  qDebug() << Q_FUNC_INFO << stanza.tag()->xml().c_str();
  // Ignore stanzas from anyone else
  if (stanza.from().bareJID() != client_->jid().bareJID()) {
    return false;
  }

  gloox::Tag* xrme = stanza.tag()->findChild("xrme");

  if (xrme->hasChild("get_artists")) {
    GetArtists();
  } else {
    qWarning() << "Unknown command received from"
               << stanza.from().resource().c_str()
               << stanza.tag()->xml().c_str();
    return false;
  }

  return true;
}

} // namespace xrme
