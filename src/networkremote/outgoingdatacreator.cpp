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

#include "outgoingdatacreator.h"

#include <cmath>

#include "networkremote.h"
#include "core/logging.h"
#include "core/timeconstants.h"

OutgoingDataCreator::OutgoingDataCreator(Application* app)
  : app_(app)
{
  // Create Keep Alive Timer
  keep_alive_timer_ = new QTimer(this);
  connect(keep_alive_timer_, SIGNAL(timeout()), this, SLOT(SendKeepAlive()));
  keep_alive_timeout_ = 10000;

  // Create the song position timer
  track_position_timer_ = new QTimer(this);
  connect(track_position_timer_, SIGNAL(timeout()), this, SLOT(UpdateTrackPosition()));
}

OutgoingDataCreator::~OutgoingDataCreator() {
}

void OutgoingDataCreator::SetClients(QList<RemoteClient*>* clients) {
  clients_ = clients;
  // After we got some clients, start the keep alive timer
  // Default: every 10 seconds
  keep_alive_timer_->start(keep_alive_timeout_);
}

void OutgoingDataCreator::SendDataToClients(pb::remote::Message* msg) {
  // Check if we have clients to send data to
  if (clients_->empty()) {
    return;
  }

  // Send the default version
  msg->set_version(msg->default_instance().version());

  RemoteClient* client;
  foreach(client, *clients_) {
    // Check if the client is still active
    if (client->State() == QTcpSocket::ConnectedState) {
      client->SendData(msg);
    } else {
      clients_->removeAt(clients_->indexOf(client));
      delete client;
    }
  }
}

void OutgoingDataCreator::SendClementineInfo() {
  // Create the general message and set the message type
  pb::remote::Message msg;
  msg.set_type(pb::remote::INFO);

  // Now add the message specific data
  pb::remote::ResponseClementineInfo* info =
      msg.mutable_response_clementine_info();
  SetEngineState(info);

  QString version = QString("%1 %2").arg(QCoreApplication::applicationName(),
                                         QCoreApplication::applicationVersion());
  info->set_version(version.toAscii());
  SendDataToClients(&msg);
}

void OutgoingDataCreator::SetEngineState(pb::remote::ResponseClementineInfo* msg) {
  switch(app_->player()->GetState()) {
    case Engine::Idle:    msg->set_state(pb::remote::Idle);
                          break;
    case Engine::Empty:   msg->set_state(pb::remote::Empty);
                          break;
    case Engine::Playing: msg->set_state(pb::remote::Playing);
                          break;
    case Engine::Paused:  msg->set_state(pb::remote::Paused);
                          break;
  }
}

void OutgoingDataCreator::SendAllPlaylists() {
  // Get all Playlists
  QList<Playlist*> app_playlists = app_->playlist_manager()->GetAllPlaylists();
  int active_playlist = app_->playlist_manager()->active_id();

  // Create message
  pb::remote::Message msg;
  msg.set_type(pb::remote::PLAYLISTS);

  pb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();

  // Get all playlists, even ones that are hidden in the UI.
  foreach (const PlaylistBackend::Playlist& p,
           app_->playlist_backend()->GetAllPlaylists()) {
    bool playlist_open = app_->playlist_manager()->IsPlaylistOpen(p.id);
    int item_count = playlist_open ?
                        app_playlists.at(p.id)->rowCount() : 0;

    // Create a new playlist
    pb::remote::Playlist* playlist = playlists->add_playlist();
    playlist->set_name(DataCommaSizeFromQString(p.name));
    playlist->set_id(p.id);
    playlist->set_active((p.id == active_playlist));
    playlist->set_item_count(item_count);
    playlist->set_closed(!playlist_open);
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendAllActivePlaylists() {
  // Get all Playlists
  QList<Playlist*> app_playlists = app_->playlist_manager()->GetAllPlaylists();
  int active_playlist = app_->playlist_manager()->active_id();

  // Create message
  pb::remote::Message msg;
  msg.set_type(pb::remote::PLAYLISTS);

  pb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();

  QListIterator<Playlist*> it(app_playlists);
  while(it.hasNext()) {
    // Get the next Playlist
    Playlist* p = it.next();
    QString playlist_name = app_->playlist_manager()->GetPlaylistName(p->id());

    // Create a new playlist
    pb::remote::Playlist* playlist = playlists->add_playlist();
    playlist->set_name(DataCommaSizeFromQString(playlist_name));
    playlist->set_id(p->id());
    playlist->set_active((p->id() == active_playlist));
    playlist->set_item_count(p->rowCount());
    playlist->set_closed(false);
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::ActiveChanged(Playlist* playlist) {
  // Send the tracks of the active playlist
  SendPlaylistSongs(playlist->id());

  // Send the changed message after sending the playlist songs
  pb::remote::Message msg;
  msg.set_type(pb::remote::ACTIVE_PLAYLIST_CHANGED);
  msg.mutable_response_active_changed()->set_id(playlist->id());
  SendDataToClients(&msg);
}

void OutgoingDataCreator::PlaylistAdded(int id, const QString& name) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistDeleted(int id) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistClosed(int id) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistRenamed(int id, const QString& new_name) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::SendFirstData(bool send_playlist_songs) {
  // First Send the current song
  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (!item) {
    qLog(Info) << "No current item found!";
  }

  CurrentSongChanged(current_song_, current_uri_, current_image_);

  // then the current volume
  VolumeChanged(app_->player()->GetVolume());

  // And the current track position
  UpdateTrackPosition();

  // And the current playlists
  SendAllActivePlaylists();

  // Send the tracks of the active playlist
  if (send_playlist_songs) {
    SendPlaylistSongs(app_->playlist_manager()->active_id());
  }

  // Send the current random and repeat mode
  SendShuffleMode(app_->playlist_manager()->sequence()->shuffle_mode());
  SendRepeatMode(app_->playlist_manager()->sequence()->repeat_mode());

  // We send all first data
  pb::remote::Message msg;
  msg.set_type(pb::remote::FIRST_DATA_SENT_COMPLETE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::CurrentSongChanged(const Song& song, const QString& uri, const QImage& img) {
  current_song_  = song;
  current_uri_   = uri;
  current_image_ = img;

  if (!clients_->empty()) {
    // Create the message
    pb::remote::Message msg;
    msg.set_type(pb::remote::CURRENT_METAINFO);

    // If there is no song, create an empty node, otherwise fill it with data
    int i = app_->playlist_manager()->active()->current_row();
    CreateSong(
        current_song_, img, i,
        msg.mutable_response_current_metadata()->mutable_song_metadata());

    SendDataToClients(&msg);
  }
}

void OutgoingDataCreator::CreateSong(
    const Song& song,
    const QImage& art,
    const int index,
    pb::remote::SongMetadata* song_metadata) {
  if (song.is_valid()) {
    song_metadata->set_id(song.id());
    song_metadata->set_index(index);
    song_metadata->set_title(DataCommaSizeFromQString(song.PrettyTitle()));
    song_metadata->set_artist(DataCommaSizeFromQString(song.artist()));
    song_metadata->set_album(DataCommaSizeFromQString(song.album()));
    song_metadata->set_albumartist(DataCommaSizeFromQString(song.albumartist()));
    song_metadata->set_length(song.length_nanosec() / kNsecPerSec);
    song_metadata->set_pretty_length(DataCommaSizeFromQString(song.PrettyLength()));
    song_metadata->set_genre(DataCommaSizeFromQString(song.genre()));
    song_metadata->set_pretty_year(DataCommaSizeFromQString(song.PrettyYear()));
    song_metadata->set_track(song.track());
    song_metadata->set_disc(song.disc());
    song_metadata->set_playcount(song.playcount());

    // Append coverart
    if (!art.isNull()) {
      QImage small;
      // Check if we resize the image
      if (art.width() > 1000) {
        small = art.scaled(1000, 1000, Qt::KeepAspectRatio);
      } else {
        small = art;
      }

      // Read the image in a buffer and compress it
      QByteArray data;
      QBuffer buf(&data);
      buf.open(QIODevice::WriteOnly);
      small.save(&buf, "JPG");

      // Append the Data in the protocol buffer
      song_metadata->set_art(data.constData(), data.size());
    }
  }
}


void OutgoingDataCreator::VolumeChanged(int volume) {
  // Create the message
  pb::remote::Message msg;
  msg.set_type(pb::remote::SET_VOLUME);
  msg.mutable_request_set_volume()->set_volume(volume);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendPlaylistSongs(int id) {
  // Get the PlaylistQByteArray(data.data(), data.size()
  Playlist* playlist = app_->playlist_manager()->playlist(id);
  if(!playlist) {
    qLog(Info) << "Could not find playlist with id = " << id;
    return;
  }

  // Create the message and the playlist
  pb::remote::Message msg;
  msg.set_type(pb::remote::PLAYLIST_SONGS);

  // Create the Response message
  pb::remote::ResponsePlaylistSongs* pb_response_playlist_songs =
                                     msg.mutable_response_playlist_songs();

  // Create a new playlist
  pb::remote::Playlist* pb_playlist =
      pb_response_playlist_songs->mutable_requested_playlist();
  pb_playlist->set_id(id);

  // Send all songs
  int index = 0;
  SongList song_list = playlist->GetAllSongs();
  QListIterator<Song> it(song_list);
  QImage null_img;
  while(it.hasNext()) {
    Song song   = it.next();
    pb::remote::SongMetadata* pb_song = pb_response_playlist_songs->add_songs();
    CreateSong(song, null_img, index, pb_song);
    ++index;
  }
  SendDataToClients(&msg);
}

void OutgoingDataCreator::PlaylistChanged(Playlist* playlist) {
  // If a playlist changed, then send the new songs to the client
  SendPlaylistSongs(playlist->id());
}

void OutgoingDataCreator::StateChanged(Engine::State state) {
  // Send state only if it changed
  // When selecting next song, StateChanged is emitted, but we already know
  // that we are playing
  if (state == last_state_) {
    return;
  }
  last_state_ = state;

  pb::remote::Message msg;

  switch (state) {
  case Engine::Playing: msg.set_type(pb::remote::PLAY);
                        track_position_timer_->start(1000);
                        break;
  case Engine::Paused:  msg.set_type(pb::remote::PAUSE);
                        track_position_timer_->stop();
                        break;
  case Engine::Empty:   msg.set_type(pb::remote::STOP); // Empty is called when player stopped
                        track_position_timer_->stop();
                        break;
  default:              msg.set_type(pb::remote::STOP);
                        track_position_timer_->stop();
                        break;
  };

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendRepeatMode(PlaylistSequence::RepeatMode mode) {
  pb::remote::Message msg;
  msg.set_type(pb::remote::REPEAT);

  switch (mode) {
  case PlaylistSequence::Repeat_Off:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Off);
       break;
  case PlaylistSequence::Repeat_Track:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Track);
       break;
  case PlaylistSequence::Repeat_Album:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Album);
       break;
  case PlaylistSequence::Repeat_Playlist:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Playlist);
       break;
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendShuffleMode(PlaylistSequence::ShuffleMode mode) {
  pb::remote::Message msg;
  msg.set_type(pb::remote::SHUFFLE);

  switch (mode) {
  case PlaylistSequence::Shuffle_Off:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_Off);
       break;
  case PlaylistSequence::Shuffle_All:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_All);
       break;
  case PlaylistSequence::Shuffle_InsideAlbum:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_InsideAlbum);
       break;
  case PlaylistSequence::Shuffle_Albums:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_Albums);
       break;
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendKeepAlive() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::KEEP_ALIVE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::UpdateTrackPosition() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::UPDATE_TRACK_POSITION);

  const int position = std::floor(
    float(app_->player()->engine()->position_nanosec()) / kNsecPerSec + 0.5);

  msg.mutable_response_update_track_position()->set_position(position);

  SendDataToClients(&msg);
}

void OutgoingDataCreator::DisconnectAllClients() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::DISCONNECT);
  msg.mutable_response_disconnect()->set_reason_disconnect(pb::remote::Server_Shutdown);
  SendDataToClients(&msg);
}
