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

#include <algorithm>

#include "core/logging.h"
#include "engines/enginebase.h"
#include "internet/internetmodel.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "playlist/playlist.h"

#ifdef HAVE_LIBLASTFM
# include "internet/lastfmservice.h"
#endif

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
  connect(this, SIGNAL(StopAfterCurrent()),
          app_->player(), SLOT(StopAfterCurrent()));
  connect(this, SIGNAL(Next()),
          app_->player(), SLOT(Next()));
  connect(this, SIGNAL(Previous()),
          app_->player(), SLOT(Previous()));
  connect(this, SIGNAL(SetVolume(int)),
          app_->player(), SLOT(SetVolume(int)));
  connect(this, SIGNAL(PlayAt(int,Engine::TrackChangeFlags,bool)),
          app_->player(), SLOT(PlayAt(int,Engine::TrackChangeFlags,bool)));
  connect(this, SIGNAL(SeekTo(int)),
          app_->player(), SLOT(SeekTo(int)));

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
  connect(this, SIGNAL(InsertUrls(int, const QList<QUrl>&, int, bool, bool)),
          app_->playlist_manager(),
          SLOT(InsertUrls(int, const QList<QUrl>&, int, bool, bool)));
  connect(this, SIGNAL(RemoveSongs(int, const QList<int>&)),
          app_->playlist_manager(),
          SLOT(RemoveItemsWithoutUndo(int, const QList<int>&)));
  connect(this, SIGNAL(Open(int)),
          app_->playlist_manager(), SLOT(Open(int)));
  connect(this, SIGNAL(Close(int)),
          app_->playlist_manager(), SLOT(Close(int)));

  connect(this, SIGNAL(RateCurrentSong(double)),
          app_->playlist_manager(), SLOT(RateCurrentSong(double)));

#ifdef HAVE_LIBLASTFM
  connect(this, SIGNAL(Love()),
          InternetModel::Service<LastFMService>(), SLOT(Love()));
  connect(this, SIGNAL(Ban()),
          InternetModel::Service<LastFMService>(), SLOT(Ban()));
#endif
}

IncomingDataParser::~IncomingDataParser() {
}

bool IncomingDataParser::close_connection() {
  return close_connection_;
}

void IncomingDataParser::Parse(const pb::remote::Message& msg) {
  close_connection_  = false;

  RemoteClient* client = qobject_cast<RemoteClient*>(sender());

  // Now check what's to do
  switch (msg.type()) {
    case pb::remote::CONNECT:
      ClientConnect(msg);
      break;
    case pb::remote::DISCONNECT:
      close_connection_ = true;
      break;
    case pb::remote::REQUEST_PLAYLISTS:
      SendPlaylists(msg);
      break;
    case pb::remote::REQUEST_PLAYLIST_SONGS:
      GetPlaylistSongs(msg);
      break;
    case pb::remote::SET_VOLUME:
      emit SetVolume(msg.request_set_volume().volume());
      break;
    case pb::remote::PLAY:
      emit Play();
      break;
    case pb::remote::PLAYPAUSE:
      emit PlayPause();
      break;
    case pb::remote::PAUSE:
      emit Pause();
      break;
    case pb::remote::STOP:
      emit Stop();
      break;
    case pb::remote::STOP_AFTER:
      emit StopAfterCurrent();
      break;
    case pb::remote::NEXT:
      emit Next();
      break;
    case pb::remote::PREVIOUS:
      emit Previous();
      break;
    case pb::remote::CHANGE_SONG:
      ChangeSong(msg);
      break;
    case pb::remote::SHUFFLE_PLAYLIST:
      emit ShuffleCurrent();
      break;
    case pb::remote::REPEAT:
      SetRepeatMode(msg.repeat());
      break;
    case pb::remote::SHUFFLE:
      SetShuffleMode(msg.shuffle());
      break;
    case pb::remote::SET_TRACK_POSITION:
      emit SeekTo(msg.request_set_track_position().position());
      break;
    case pb::remote::INSERT_URLS:
      InsertUrls(msg);
      break;
    case pb::remote::REMOVE_SONGS:
      RemoveSongs(msg);
      break;
    case pb::remote::OPEN_PLAYLIST:
      OpenPlaylist(msg);
      break;
    case pb::remote::CLOSE_PLAYLIST:
      ClosePlaylist(msg);
      break;
    case pb::remote::LOVE:
      emit Love();
      break;
    case pb::remote::BAN:
      emit Ban();
      break;
    case pb::remote::GET_LYRICS:
      emit GetLyrics();
      break;
    case pb::remote::DOWNLOAD_SONGS:
      emit SendSongs(msg.request_download_songs(), client);
      break;
    case pb::remote::SONG_OFFER_RESPONSE:
      emit ResponseSongOffer(client, msg.response_song_offer().accepted());
      break;
    case pb::remote::GET_LIBRARY:
      emit SendLibrary(client);
      break;
    case pb::remote::RATE_SONG:
      RateSong(msg);
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

void IncomingDataParser::InsertUrls(const pb::remote::Message& msg) {
  const pb::remote::RequestInsertUrls& request = msg.request_insert_urls();

  // Extract urls
  QList<QUrl> urls;
  for (auto it = request.urls().begin(); it != request.urls().end(); ++it) {
    urls << QUrl(QString::fromStdString(*it));
  }

  // Insert the urls
  emit InsertUrls(request.playlist_id(), urls, request.position(),
                  request.play_now(), request.enqueue());
}

void IncomingDataParser::RemoveSongs(const pb::remote::Message& msg) {
    const pb::remote::RequestRemoveSongs& request = msg.request_remove_songs();

    // Extract urls
    QList<int> songs;
    for (int i = 0; i<request.songs().size();i++) {
        songs.append(request.songs(i));
    }

    // Insert the urls
    emit RemoveSongs(request.playlist_id(), songs);
}

void IncomingDataParser::ClientConnect(const pb::remote::Message& msg) {

  // Always sned the Clementine infos
  emit SendClementineInfo();

  // Check if we should send the first data
  if (!msg.request_connect().has_send_playlist_songs() // legacy
   || msg.request_connect().send_playlist_songs()) {
    emit SendFirstData(true);
  } else {
    emit SendFirstData(false);
  }
}

void IncomingDataParser::SendPlaylists(const pb::remote::Message &msg) {
  if (!msg.has_request_playlists()
   || !msg.request_playlists().include_closed()) {
    emit SendAllActivePlaylists();
  } else {
    emit SendAllPlaylists();
  }
}

void IncomingDataParser::OpenPlaylist(const pb::remote::Message &msg) {
  emit Open(msg.request_open_playlist().playlist_id());
}

void IncomingDataParser::ClosePlaylist(const pb::remote::Message &msg) {
  emit Close(msg.request_close_playlist().playlist_id());
}

void IncomingDataParser::RateSong(const pb::remote::Message &msg) {
  double rating = (double) msg.request_rate_song().rating();
  emit RateCurrentSong(rating);
}
