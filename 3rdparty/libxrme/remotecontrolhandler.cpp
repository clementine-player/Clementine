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

#include "mediaplayerhandler.h"
#include "remotecontrolinterface.h"
#include "remotecontrolhandler.h"

#include <QImage>
#include <QtDebug>

#include <gloox/client.h>
#include <gloox/disco.h>

namespace xrme {

RemoteControlHandler::RemoteControlHandler(RemoteControlInterface* interface)
    : interface_(interface) {
  interface_->Attach(this);
}

void RemoteControlHandler::Init(Connection* connection, gloox::Client* client) {
  Handler::Init(connection, client);

  client->registerIqHandler(this, kXmlnsXrmeRemoteControl);
  client->disco()->addFeature(kXmlnsXrmeRemoteControl);
}

void RemoteControlHandler::SendIQ(const QString& jid_resource,
                                  gloox::StanzaSubType type,
                                  const QString& command) {
  if (!client_) {
    return;
  }

  gloox::JID to(client_->jid());
  to.setResource(jid_resource.toUtf8().constData());

  gloox::Tag* stanza = gloox::Stanza::createIqStanza(
        to, client_->getID(), type, kXmlnsXrmeMediaPlayer);
  gloox::Tag* c = new gloox::Tag(stanza, command.toUtf8().constData());
  c->addAttribute("xmlns", kXmlnsXrmeMediaPlayer);

  client_->send(stanza);
}

void RemoteControlHandler::PlayPause(const QString& jid_resource) {
  SendIQ(jid_resource, gloox::StanzaIqSet, "playpause");
}

void RemoteControlHandler::Stop(const QString& jid_resource) {
  SendIQ(jid_resource, gloox::StanzaIqSet, "stop");
}

void RemoteControlHandler::Next(const QString& jid_resource) {
  SendIQ(jid_resource, gloox::StanzaIqSet, "next");
}

void RemoteControlHandler::Previous(const QString& jid_resource) {
  SendIQ(jid_resource, gloox::StanzaIqSet, "previous");
}

void RemoteControlHandler::QueryState(const QString& jid_resource) {
  SendIQ(jid_resource, gloox::StanzaIqGet, "querystate");
}

int RemoteControlHandler::ParseInt(gloox::Tag* tag, const char* attribute_name) {
  return ParseString(tag, attribute_name).toInt();
}

double RemoteControlHandler::ParseDouble(gloox::Tag* tag, const char* attribute_name) {
  return ParseString(tag, attribute_name).toFloat();
}

QString RemoteControlHandler::ParseString(gloox::Tag* tag, const char* attribute_name) {
  return QString::fromUtf8(tag->findAttribute(attribute_name).c_str());
}

bool RemoteControlHandler::handleIq(gloox::Stanza* stanza) {
  // Ignore stanzas from anyone else
  if (stanza->from().bareJID() != client_->jid().bareJID()) {
    return false;
  }

  QString resource = QString::fromUtf8(stanza->from().resource().c_str());

  qDebug() << resource << stanza->xml().c_str();

  gloox::Tag* state = stanza->findChild("state");
  if (state) {
    gloox::Tag* metadata = state->findChild("metadata");
    if (metadata) {
      State s;
      s.playback_state    = State::PlaybackState(ParseInt(state, "playback_state"));
      s.position_millisec = ParseInt(state, "position_millisec");
      s.volume            = ParseDouble(state, "volume");
      s.can_go_next       = ParseInt(state, "can_go_next");
      s.can_go_previous   = ParseInt(state, "can_go_previous");
      s.can_seek          = ParseInt(state, "can_seek");

      s.metadata.title    = ParseString(metadata, "title");
      s.metadata.artist   = ParseString(metadata, "artist");
      s.metadata.album    = ParseString(metadata, "album");
      s.metadata.albumartist = ParseString(metadata, "albumartist");
      s.metadata.composer = ParseString(metadata, "composer");
      s.metadata.genre    = ParseString(metadata, "genre");
      s.metadata.track    = ParseInt(metadata, "track");
      s.metadata.disc     = ParseInt(metadata, "disc");
      s.metadata.year     = ParseInt(metadata, "year");
      s.metadata.length_millisec = ParseInt(metadata, "length_millisec");
      s.metadata.rating   = ParseDouble(metadata, "rating");

      interface_->StateChanged(resource, s);
    }
  }

  gloox::Tag* album_art = stanza->findChild("album_art");
  if (album_art) {
    QByteArray data(album_art->cdata().c_str(), album_art->cdata().size());

    QImage image;
    if (!data.isEmpty()) {
      image.loadFromData(QByteArray::fromBase64(data), "JPEG");
    }

    interface_->AlbumArtChanged(resource, image);
  }

  return state || album_art;
}

} // namespace xrme
