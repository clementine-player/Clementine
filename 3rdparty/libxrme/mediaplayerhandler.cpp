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
#include "mediaplayerhandler.h"
#include "mediaplayerinterface.h"
#include "remotecontrolhandler.h"

#include <QBuffer>
#include <QtDebug>

#include <gloox/client.h>
#include <gloox/disco.h>

namespace xrme {

MediaPlayerHandler::MediaPlayerHandler(MediaPlayerInterface* interface)
    : interface_(interface) {
  interface_->Attach(this);
}

void MediaPlayerHandler::StateChanged() {
  if (!connection_) {
    return;
  }

  State s = interface_->state();

  foreach (const Connection::Peer& peer, connection_->peers(Connection::Peer::RemoteControl)) {
    gloox::JID to(client_->jid().bareJID());
    to.setResource(peer.jid_resource_.toUtf8().constData());

    gloox::IQ iq(gloox::IQ::Set, to, client_->getID());
    gloox::Tag* tag = iq.tag();
    gloox::Tag* c = new gloox::Tag(tag, "xrme");
    c->setXmlns(kXmlnsXrmeRemoteControl);
    gloox::Tag* state = new gloox::Tag(c, "state");
    state->addAttribute("playback_state", s.playback_state);
    state->addAttribute("position_millisec", s.position_millisec);
    state->addAttribute(
        "volume", QString::number(s.volume, 'f').toUtf8().constData());
    state->addAttribute("can_go_next", s.can_go_next ? 1 : 0);
    state->addAttribute("can_go_previous", s.can_go_previous ? 1 : 0);
    state->addAttribute("can_seek", s.can_seek ? 1 : 0);

    gloox::Tag* metadata = new gloox::Tag(state, "metadata");
    metadata->addAttribute("title", s.metadata.title.toUtf8().constData());
    metadata->addAttribute("artist", s.metadata.artist.toUtf8().constData());
    metadata->addAttribute("album", s.metadata.album.toUtf8().constData());
    metadata->addAttribute(
        "albumartist", s.metadata.albumartist.toUtf8().constData());
    metadata->addAttribute(
        "composer", s.metadata.composer.toUtf8().constData());
    metadata->addAttribute("genre", s.metadata.genre.toUtf8().constData());
    metadata->addAttribute("track", s.metadata.track);
    metadata->addAttribute("disc", s.metadata.disc);
    metadata->addAttribute("year", s.metadata.year);
    metadata->addAttribute("length_millisec", s.metadata.length_millisec);
    metadata->addAttribute(
        "rating",
        QString::number(s.metadata.rating, 'f').toUtf8().constData());

    client_->send(tag);
  }
}

void MediaPlayerHandler::AlbumArtChanged() {
  if (!connection_) {
    return;
  }

  QImage image = interface_->album_art();

  // Scale the image down if it's too big
  if (!image.isNull() && (image.width() > kMaxAlbumArtSize ||
                          image.height() > kMaxAlbumArtSize)) {
    image = image.scaled(kMaxAlbumArtSize,kMaxAlbumArtSize,
                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }

  // Write the image data
  QByteArray image_data;
  if (!image.isNull()) {
    QBuffer buffer(&image_data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG");
  }

  // Convert to base64
  QByteArray image_data_base64 = image_data.toBase64();

  // Send the IQs
  foreach (const Connection::Peer& peer, connection_->peers(Connection::Peer::RemoteControl)) {
    gloox::JID to(client_->jid().bareJID());
    to.setResource(peer.jid_resource_.toUtf8().constData());

    gloox::IQ iq(gloox::IQ::Set, to, client_->getID());
    gloox::Tag* tag = iq.tag();
    gloox::Tag* c = new gloox::Tag(tag, "xrme");
    c->setXmlns(kXmlnsXrmeRemoteControl);
    gloox::Tag* album_art = new gloox::Tag(c, "album_art");
    album_art->setCData(image_data_base64.constData());

    client_->send(tag);
  }
}

void MediaPlayerHandler::Init(Connection* connection, gloox::Client* client) {
  Handler::Init(connection, client);

  client->registerIqHandler(
      this, MediaPlayerExtension::kExtensionType);
  client->disco()->addFeature(kXmlnsXrmeMediaPlayer);
}

bool MediaPlayerHandler::handleIq(const gloox::IQ& stanza) {
  qDebug() << Q_FUNC_INFO << stanza.tag()->xml().c_str();
  // Ignore stanzas from anyone else
  if (stanza.from().bareJID() != client_->jid().bareJID()) {
    return false;
  }

  gloox::Tag* xrme = stanza.tag()->findChild("xrme");

  if (xrme->hasChild("playpause")) {
    interface_->PlayPause();
  } else if (xrme->hasChild("stop")) {
    interface_->Stop();
  } else if (xrme->hasChild("previous")) {
    interface_->Previous();
  } else if (xrme->hasChild("next")) {
    interface_->Next();
  } else if (xrme->hasChild("querystate")) {
    StateChanged();
    AlbumArtChanged();
  } else {
    qWarning() << "Unknown command received from"
               << stanza.from().resource().c_str()
               << stanza.tag()->xml().c_str();
    return false;
  }

  return true;
}

} // namespace xrme
