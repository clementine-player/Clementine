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
#include "playlist/playlistsequence.h"

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

  // For some connects we have to wait for the playlistmanager
  // to be initialized
  connect(app_->playlist_manager(), SIGNAL(PlaylistManagerInitialized()),
          this, SLOT(PlaylistManagerInitialized()));
}

void IncomingDataParser::PlaylistManagerInitialized() {
  connect(this, SIGNAL(SetActivePlaylist(int)),
          app_->playlist_manager(), SLOT(SetActivePlaylist(int)));
  connect(this, SIGNAL(ShuffleCurrent()),
          app_->playlist_manager(), SLOT(ShuffleCurrent()));
  connect(this, SIGNAL(SetRepeatMode(PlaylistSequence::RepeatMode)),
          app_->playlist_manager()->sequence(),
          SLOT(SetRepeatMode(PlaylistSequence::RepeatMode)));
  connect(this, SIGNAL(SetShuffleMode(PlaylistSequence::ShuffleMode)),
          app_->playlist_manager()->sequence(),
          SLOT(SetShuffleMode(PlaylistSequence::ShuffleMode)));
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
  switch (msg.type()) {
    case pb::remote::CONNECT:     emit SendClementineInfo();
                                  emit SendFirstData();
                                  break;
    case pb::remote::DISCONNECT:  close_connection_ = true;
                                  break;
    case pb::remote::REQUEST_PLAYLISTS:       emit SendAllPlaylists();
                                              break;
    case pb::remote::REQUEST_PLAYLIST_SONGS:  GetPlaylistSongs(msg);
                                              break;
    case pb::remote::SET_VOLUME:  emit SetVolume(msg.request_set_volume().volume());
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
    case pb::remote::PREVIOUS:    emit Previous();
                                  break;
    case pb::remote::CHANGE_SONG: ChangeSong(msg);
                                  break;
    case pb::remote::SHUFFLE_PLAYLIST:        emit ShuffleCurrent();
                                              break;
    case pb::remote::REPEAT:      SetRepeatMode(msg.repeat());
                                  break;
    case pb::remote::SHUFFLE:     SetShuffleMode(msg.shuffle());
                                  break;
    default: break;
  }
}

void IncomingDataParser::GetPlaylistSongs(const pb::remote::Message& msg) {
  emit SendPlaylistSongs(msg.request_playlist_songs().id());
}

void IncomingDataParser::ChangeSong(const pb::remote::Message& msg) {
  // Get the first entry and check if there is a song
  const pb::remote::RequestChangeSong& request = msg.request_change_song();

  // Check if we need to change the playlist
  if (request.playlist_id() != app_->playlist_manager()->active_id()) {
    emit SetActivePlaylist(request.playlist_id());
  }

  // Play the selected song
  emit PlayAt(request.song_index(), Engine::Manual, false);
}

void IncomingDataParser::SetRepeatMode(const pb::remote::Repeat& repeat) {
  switch (repeat.repeat_mode()) {
  case pb::remote::Repeat_Off:
       emit SetRepeatMode(PlaylistSequence::Repeat_Off);
       break;
  case pb::remote::Repeat_Track:
       emit SetRepeatMode(PlaylistSequence::Repeat_Track);
       break;
  case pb::remote::Repeat_Album:
       emit SetRepeatMode(PlaylistSequence::Repeat_Album);
       break;
  case pb::remote::Repeat_Playlist:
       emit SetRepeatMode(PlaylistSequence::Repeat_Playlist);
       break;
  default: break;
  }
}

void IncomingDataParser::SetShuffleMode(const pb::remote::Shuffle& shuffle) {
  switch (shuffle.shuffle_mode()) {
  case pb::remote::Shuffle_Off:
       emit SetShuffleMode(PlaylistSequence::Shuffle_Off);
       break;
  case pb::remote::Shuffle_All:
       emit SetShuffleMode(PlaylistSequence::Shuffle_All);
       break;
  case pb::remote::Shuffle_InsideAlbum:
       emit SetShuffleMode(PlaylistSequence::Shuffle_InsideAlbum);
       break;
  case pb::remote::Shuffle_Albums:
       emit SetShuffleMode(PlaylistSequence::Shuffle_Albums);
       break;
  default: break;
  }
}
