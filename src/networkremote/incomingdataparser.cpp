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

#include "incomingdataparser.h"
#include "core/logging.h"
#include "engines/enginebase.h"
#include "playlist/playlistmanager.h"

IncomingDataParser::IncomingDataParser(Application* app)
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
  connect(this, SIGNAL(ShuffleCurrent()),
          app_->playlist_manager(), SLOT(ShuffleCurrent()));
}

IncomingDataParser::~IncomingDataParser() {
}

bool IncomingDataParser::close_connection() {
  return close_connection_;
}

void IncomingDataParser::Parse(const QByteArray& data) {
  close_connection_  = false;

  // Parse the incoming data
  pb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) {
    qLog(Info) << "Couldn't parse data";
    return;
  }

  // Now check what's to do
  switch (msg.msgtype()) {
    case pb::remote::CONNECT:     emit SendClementineInfos();
                                  emit SendFirstData();
                                  break;
    case pb::remote::DISCONNECT:  close_connection_ = true;
                                  break;
    case pb::remote::REQUEST_PLAYLISTS:       emit SendAllPlaylists();
                                              break;
    case pb::remote::REQUEST_PLAYLIST_SONGS:  GetPlaylistSongs(&msg);
                                              break;
    case pb::remote::SET_VOLUME:  emit SetVolume(msg.volume());
                                  break;
    case pb::remote::PLAY:        emit Play();
                                  break;
    case pb::remote::PLAYPAUSE:   emit PlayPause();
                                  break;
    case pb::remote::PAUSE:       emit Pause();
                                  break;
    case pb::remote::STOP:        emit Stop();
                                  break;
    case pb::remote::NEXT:        emit Next();
                                  break;
    case pb::remote::PREV:        emit Previous();
                                  break;
    case pb::remote::CHANGE_SONG: ChangeSong(&msg);
                                  break;
    case pb::remote::TOGGLE_SHUFFLE:  emit ShuffleCurrent();
                                      break;
    default: break;
  }
}

void IncomingDataParser::GetPlaylistSongs(pb::remote::Message* msg) {
  // Check if we got a playlist
  if (msg->playlists_size() == 0)
  {
    return;
  }

  // Get the first entry and send the songs
  pb::remote::Playlist playlist = msg->playlists(0);
  emit SendPlaylistSongs(playlist.id());
}

void IncomingDataParser::ChangeSong(pb::remote::Message* msg) {
  // Check if we got a song
  if (msg->playlists_size() == 0) {
    return;
  }

  // Get the first entry and check if there is a song
  pb::remote::Playlist playlist = msg->playlists(0);
  if (playlist.songs_size() == 0) {
    return;
  }

  pb::remote::SongMetadata song = playlist.songs(0);

  // Check if we need to change the playlist
  if (playlist.id() != app_->playlist_manager()->active_id()) {
    emit SetActivePlaylist(playlist.id());
  }

  // Play the selected song
  emit PlayAt(song.index(), Engine::Manual, false);
}
