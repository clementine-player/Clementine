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

#include "remotexmltags.h"

// Nodes
const QString RemoteXmlTags::ROOT       = "ClementineRemote"; // The root node of every xml
const QString RemoteXmlTags::ACTION     = "Action"; // The Node where the action is specified
const QString RemoteXmlTags::SONG       = "Song"; // The Node for song metadata
const QString RemoteXmlTags::INFOS      = "Infos";
const QString RemoteXmlTags::VERSION    = "Version";

// Playlist
const QString RemoteXmlTags::PLAYLIST        = "Playlist";
const QString RemoteXmlTags::PLAYLIST_NAME   = "name";
const QString RemoteXmlTags::PLAYLIST_ITEMS  = "items";
const QString RemoteXmlTags::PLAYLIST_ACTIVE = "active";

// Attributes
const QString RemoteXmlTags::ID       = "id";
const QString RemoteXmlTags::INDEX    = "index";
const QString RemoteXmlTags::CURRENT_ROW = "currentRow";

// Actions
const QString RemoteXmlTags::CLIENT_CONNTECTED      = "Connect"; // A client wants to connect
const QString RemoteXmlTags::CLIENT_DISCONNECT      = "Disconnect"; // A Client wants to disconnect
const QString RemoteXmlTags::SEND_INFOS             = "Info";
const QString RemoteXmlTags::SEND_METAINFOS         = "CurrentMetainfos"; // Sends the metainfos of the current song
const QString RemoteXmlTags::SEND_PLAYLISTS         = "Playlists";
const QString RemoteXmlTags::SEND_PLAYLIST_SONGS    = "PlaylistSongs";
const QString RemoteXmlTags::REQUEST_PLAYLISTS      = "GetPlaylists";
const QString RemoteXmlTags::REQUEST_PLAYLIST_SONGS = "GetPlaylistSongs";
const QString RemoteXmlTags::PLAY                   = "Play";
const QString RemoteXmlTags::PLAYPAUSE              = "PlayPause";
const QString RemoteXmlTags::PAUSE                  = "Pause";
const QString RemoteXmlTags::STOP                   = "Stop";
const QString RemoteXmlTags::NEXT                   = "Next";
const QString RemoteXmlTags::PREV                   = "Prev";
const QString RemoteXmlTags::CHANGE_SONG            = "ChangeSong";
const QString RemoteXmlTags::SET_VOLUME             = "SetVolume";
const QString RemoteXmlTags::KEEP_ALIVE             = "KeepAlive";

// Tags in Song
const QString RemoteXmlTags::SONG_TITLE       = "Title";
const QString RemoteXmlTags::SONG_ARTIST      = "Artist";
const QString RemoteXmlTags::SONG_ALBUM       = "Album";
const QString RemoteXmlTags::SONG_ALBUMARTIST = "AlbumArtist";
const QString RemoteXmlTags::SONG_LENGTH      = "Length";
const QString RemoteXmlTags::SONG_GENRE       = "Genre";
const QString RemoteXmlTags::SONG_YEAR        = "Year";
const QString RemoteXmlTags::SONG_TRACK       = "Track";
const QString RemoteXmlTags::SONG_DISC        = "Disc";
const QString RemoteXmlTags::SONG_PLAYCOUNT   = "Playcount";
const QString RemoteXmlTags::SONG_ART         = "Art";
const QString RemoteXmlTags::VOLUME           = "Volume";

RemoteXmlTags::RemoteXmlTags() {
}

RemoteXmlTags::~RemoteXmlTags() {

}

