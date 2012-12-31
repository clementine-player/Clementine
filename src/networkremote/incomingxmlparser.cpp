/* This file is part of Clementine.
   Copyright 2012, Andreas Muttscheller <asfa194@gmail.com>

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

#include "incomingxmlparser.h"
#include "remotexmltags.h"
#include "core/logging.h"
#include "engines/enginebase.h"
#include "playlist/playlistmanager.h"

IncomingXmlParser::IncomingXmlParser(Application* app)
  :app_(app)
{
  // Connect all the signals
  // due the player is in a different thread, we cannot access these functions directly
  connect(this, SIGNAL(Play()),
          app_->player(), SLOT(Play()));
  connect(this, SIGNAL(PlayPause()),
          app_->player(), SLOT(PlayPause()));
  connect(this, SIGNAL(Pause()),
          app_->player(), SLOT(Pause()));
  connect(this, SIGNAL(Stop()),
          app_->player(), SLOT(Stop()));
  connect(this, SIGNAL(Next()),
          app_->player(), SLOT(Next()));
  connect(this, SIGNAL(Previous()),
          app_->player(), SLOT(Previous()));
  connect(this, SIGNAL(SetVolume(int)),
          app_->player(), SLOT(SetVolume(int)));
  connect(this, SIGNAL(PlayAt(int,Engine::TrackChangeFlags,bool)),
          app_->player(), SLOT(PlayAt(int,Engine::TrackChangeFlags,bool)));
  connect(this, SIGNAL(SetActivePlaylist(int)),
          app_->playlist_manager(), SLOT(SetActivePlaylist(int)));
}

IncomingXmlParser::~IncomingXmlParser() {
}

bool IncomingXmlParser::CloseConnection() {
  return close_connection_;
}

void IncomingXmlParser::Parse(QString *xml_data) {
  close_connection_ = false;

  // Load the xml data
  QDomDocument doc;
  doc.setContent(*xml_data);

  QDomElement root = doc.documentElement();
  if (root.tagName() != RemoteXmlTags::ROOT) {
    qLog(Info) << "Received invalid XML data";
    qLog(Debug) << xml_data;
    return;
  }

  // The first child must be "action". It tells us what to do now
  QDomNode child = root.firstChild();
  if (child.toElement().tagName() != RemoteXmlTags::ACTION) {
    qLog(Info) << "First node is not action! (" <<
                  child.toElement().tagName() << ")";
    return;
  }

  // Now check what's to do
  QString action = child.toElement().text();
  qLog(Debug) << "Action = " << action;
  if (action == RemoteXmlTags::CLIENT_CONNTECTED) {
    emit SendClementineInfos();
    emit SendFirstData()/* This file is part of Clementine.
       Copyright 2012, David Sansome <me@davidsansome.com>

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
    */;
  } else if (action == RemoteXmlTags::CLIENT_DISCONNECT) {
    close_connection_ = true;
  } else if (action == RemoteXmlTags::REQUEST_PLAYLISTS) {
    emit SendAllPlaylists();
  } else if (action == RemoteXmlTags::REQUEST_PLAYLIST_SONGS) {
    GetPlaylistSongs(child);
  } else if (action == RemoteXmlTags::SET_VOLUME) {
    ChangeVolume(child);
  } else if (action == RemoteXmlTags::PLAY) {
    emit Play();
  } else if (action == RemoteXmlTags::PLAYPAUSE) {
    emit PlayPause();
  } else if (action == RemoteXmlTags::PAUSE) {
    emit Pause();
  } else if (action == RemoteXmlTags::STOP) {
    emit Stop();
  } else if (action == RemoteXmlTags::NEXT) {
    emit Next();
  } else if (action == RemoteXmlTags::PREV) {
    emit Previous();
  } else if (action == RemoteXmlTags::CHANGE_SONG) {
    ChangeSong(child);
  }
}

void IncomingXmlParser::GetPlaylistSongs(QDomNode &child) {
  QDomNode c = child;
  // Get the Playlist Tag
  while (!c.isNull() && c.toElement().tagName() != RemoteXmlTags::PLAYLIST) {
    c = c.nextSibling();
  }

  // If the node was found, send a signal to the outgoingXmlCreator to create the songlist
  if (!c.isNull()) {
    QDomElement p = c.toElement();

    qLog(Debug) << "Element c = " << p.tagName() <<
                   " Attribute: " << c.toElement().attribute(RemoteXmlTags::ID);
    int id = c.toElement().attribute(RemoteXmlTags::ID).toInt();
    emit SendPlaylistSongs(id);
  }
}

void IncomingXmlParser::ChangeVolume(QDomNode& child) {
  QDomNode c = child;
  // Get the Volume Tag
  while (!c.isNull() && c.toElement().tagName() != RemoteXmlTags::VOLUME) {
    c = c.nextSibling();
  }

  // If we found it, the change the volume
  if (!c.isNull()) {
    emit SetVolume(c.toElement().text().toInt());
  }
}

void IncomingXmlParser::ChangeSong(QDomNode& child) {
  QDomNode c = child;
  // Get the Volume Tag
  while (!c.isNull() && c.toElement().tagName() != RemoteXmlTags::SONG) {
    c = c.nextSibling();
  }

  // If we found it, the change the volume
  if (!c.isNull()) {
    // first check if we need to change the active Playlist
    int selectedPlaylist = c.toElement().attribute(RemoteXmlTags::PLAYLIST).toInt();
    int selectedSong     = c.toElement().attribute(RemoteXmlTags::INDEX).toInt();
    if (selectedPlaylist != app_->playlist_manager()->active_id()) {
      emit SetActivePlaylist(selectedPlaylist);
    }
    emit PlayAt(selectedSong, Engine::Manual, false);
  }
}
