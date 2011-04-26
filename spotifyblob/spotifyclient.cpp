/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.


#include "spotifyclient.h"
#include "spotifykey.h"
#include "spotifymessages.pb.h"
#include "core/logging.h"

#include <QDir>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

SpotifyClient::SpotifyClient(QObject* parent)
  : QObject(parent),
    api_key_(QByteArray::fromBase64(kSpotifyApiKey)),
    socket_(new QTcpSocket(this)),
    session_(NULL),
    events_timer_(new QTimer(this)) {
  memset(&spotify_callbacks_, 0, sizeof(spotify_callbacks_));
  memset(&spotify_config_, 0, sizeof(spotify_config_));
  memset(&playlistcontainer_callbacks_, 0, sizeof(playlistcontainer_callbacks_));
  memset(&load_playlist_callbacks_, 0, sizeof(load_playlist_callbacks_));

  spotify_callbacks_.logged_in = &LoggedInCallback;
  spotify_callbacks_.notify_main_thread = &NotifyMainThreadCallback;
  spotify_callbacks_.log_message = &LogMessageCallback;
  spotify_callbacks_.metadata_updated = &MetadataUpdatedCallback;

  playlistcontainer_callbacks_.container_loaded = &PlaylistContainerLoadedCallback;
  playlistcontainer_callbacks_.playlist_added = &PlaylistAddedCallback;
  playlistcontainer_callbacks_.playlist_moved = &PlaylistMovedCallback;
  playlistcontainer_callbacks_.playlist_removed = &PlaylistRemovedCallback;

  load_playlist_callbacks_.playlist_state_changed = &PlaylistStateChanged;

  spotify_config_.api_version = SPOTIFY_API_VERSION;  // From libspotify/api.h
  spotify_config_.cache_location = strdup(QDir::tempPath().toLocal8Bit().constData());
  spotify_config_.settings_location = strdup(QDir::tempPath().toLocal8Bit().constData());
  spotify_config_.application_key = api_key_.constData();
  spotify_config_.application_key_size = api_key_.size();
  spotify_config_.callbacks = &spotify_callbacks_;
  spotify_config_.userdata = this;
  spotify_config_.user_agent = "Clementine Player";

  events_timer_->setSingleShot(true);
  connect(events_timer_, SIGNAL(timeout()), SLOT(ProcessEvents()));

  connect(socket_, SIGNAL(readyRead()), SLOT(SocketReadyRead()));
}

SpotifyClient::~SpotifyClient() {
  if (session_) {
    sp_session_release(session_);
  }

  free(const_cast<char*>(spotify_config_.cache_location));
  free(const_cast<char*>(spotify_config_.settings_location));
  free(const_cast<void*>(spotify_config_.application_key));
}

void SpotifyClient::Init(quint16 port) {
  qLog(Debug) << "Connecting to port" << port;

  socket_->connectToHost(QHostAddress::LocalHost, port);
}

void SpotifyClient::LoggedInCallback(sp_session* session, sp_error error) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));
  const bool success = error == SP_ERROR_OK;

  if (!success) {
    qLog(Warning) << "Failed to login" << sp_error_message(error);
  }

  me->SendLoginCompleted(success, sp_error_message(error));

  if (success) {
    sp_playlistcontainer_add_callbacks(
          sp_session_playlistcontainer(session),
          &me->playlistcontainer_callbacks_, me);
  }
}

void SpotifyClient::NotifyMainThreadCallback(sp_session* session) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));
  QMetaObject::invokeMethod(me, "ProcessEvents", Qt::QueuedConnection);
}

void SpotifyClient::ProcessEvents() {
  int next_timeout_ms;
  sp_session_process_events(session_, &next_timeout_ms);
  events_timer_->start(next_timeout_ms);
}

void SpotifyClient::LogMessageCallback(sp_session* session, const char* data) {
  qLog(Debug) << "libspotify:" << QString::fromUtf8(data).trimmed();
}

void SpotifyClient::Search(const QString& query) {
  sp_search_create(
      session_,
      query.toUtf8().constData(),
      0,   // track offset
      10,  // track count
      0,   // album offset
      10,  // album count
      0,   // artist offset
      10,  // artist count
      &SearchCompleteCallback,
      this);
}

void SpotifyClient::SearchCompleteCallback(sp_search* result, void* userdata) {
  sp_error error = sp_search_error(result);
  if (error != SP_ERROR_OK) {
    qLog(Warning) << "Search failed";
    sp_search_release(result);
    return;
  }

  int artists = sp_search_num_artists(result);
  for (int i = 0; i < artists; ++i) {
    sp_artist* artist = sp_search_artist(result, i);
    qLog(Debug) << "Found artist:" << sp_artist_name(artist);
  }

  sp_search_release(result);
}

void SpotifyClient::SocketReadyRead() {
  protobuf::SpotifyMessage message;
  if (!ReadMessage(socket_, &message)) {
    socket_->deleteLater();
    socket_ = NULL;
    return;
  }

  if (message.has_login_request()) {
    const protobuf::LoginRequest& r = message.login_request();
    Login(QStringFromStdString(r.username()), QStringFromStdString(r.password()));
  } else if (message.has_load_playlist_request()) {
    LoadPlaylist(message.load_playlist_request());
  }
}

void SpotifyClient::Login(const QString& username, const QString& password) {
  sp_error error = sp_session_create(&spotify_config_, &session_);
  if (error != SP_ERROR_OK) {
    qLog(Warning) << "Failed to create session" << sp_error_message(error);
    SendLoginCompleted(false, sp_error_message(error));
    return;
  }

  sp_session_login(session_, username.toUtf8().constData(), password.toUtf8().constData());
}

void SpotifyClient::SendLoginCompleted(bool success, const QString& error) {
  protobuf::SpotifyMessage message;

  protobuf::LoginResponse* response = message.mutable_login_response();
  response->set_success(success);
  response->set_error(DataCommaSizeFromQString(error));

  SendMessage(socket_, message);
}

void SpotifyClient::PlaylistContainerLoadedCallback(sp_playlistcontainer* pc, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);
  me->GetPlaylists();
}

void SpotifyClient::PlaylistAddedCallback(sp_playlistcontainer* pc, sp_playlist* playlist, int position, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);
  me->GetPlaylists();
}

void SpotifyClient::PlaylistMovedCallback(sp_playlistcontainer* pc, sp_playlist* playlist, int position, int new_position, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);
  me->GetPlaylists();
}

void SpotifyClient::PlaylistRemovedCallback(sp_playlistcontainer* pc, sp_playlist* playlist, int position, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);
  me->GetPlaylists();
}

void SpotifyClient::GetPlaylists() {
  protobuf::SpotifyMessage message;
  protobuf::Playlists* response = message.mutable_playlists_updated();

  sp_playlistcontainer* container = sp_session_playlistcontainer(session_);
  if (!container) {
    qLog(Warning) << "sp_session_playlistcontainer returned NULL";
    return;
  }

  const int count = sp_playlistcontainer_num_playlists(container);
  qLog(Debug) << "Playlist container has" << count << "playlists";

  for (int i=0 ; i<count ; ++i) {
    const int type = sp_playlistcontainer_playlist_type(container, i);
    sp_playlist* playlist = sp_playlistcontainer_playlist(container, i);

    qLog(Debug) << "Got playlist" << i << type << sp_playlist_name(playlist);

    if (type != SP_PLAYLIST_TYPE_PLAYLIST) {
      // Just ignore folders for now
      continue;
    }

    protobuf::Playlists::Playlist* msg = response->add_playlist();
    msg->set_index(i);
    msg->set_name(sp_playlist_name(playlist));
  }

  SendMessage(socket_, message);
}

void SpotifyClient::LoadPlaylist(const protobuf::LoadPlaylistRequest& req) {
  PendingLoadPlaylist pending_load;
  pending_load.request_ = req;
  pending_load.playlist_ = NULL;

  switch (req.type()) {
    case protobuf::LoadPlaylistRequest_Type_Inbox:
      pending_load.playlist_ = sp_session_inbox_create(session_);
      break;

    case protobuf::LoadPlaylistRequest_Type_Starred:
      pending_load.playlist_ = sp_session_starred_create(session_);
      break;

    case protobuf::LoadPlaylistRequest_Type_UserPlaylist: {
      const int index = req.user_playlist_index();
      sp_playlistcontainer* pc = sp_session_playlistcontainer(session_);

      if (pc && index <= sp_playlistcontainer_num_playlists(pc)) {
        if (sp_playlistcontainer_playlist_type(pc, index) == SP_PLAYLIST_TYPE_PLAYLIST) {
          pending_load.playlist_ = sp_playlistcontainer_playlist(pc, index);
          sp_playlist_add_ref(pending_load.playlist_);
        }
      }

      break;
    }
  }

  // A null playlist might mean the user wasn't logged in, or an invalid
  // playlist index was requested, so we'd better return an error straight away.
  if (!pending_load.playlist_) {
    qLog(Warning) << "Invalid playlist requested or not logged in";

    protobuf::SpotifyMessage message;
    protobuf::LoadPlaylistResponse* response = message.mutable_load_playlist_response();
    *response->mutable_request() = req;
    SendMessage(socket_, message);
    return;
  }

  sp_playlist_add_callbacks(pending_load.playlist_, &load_playlist_callbacks_, this);
  pending_load_playlists_ << pending_load;

  PlaylistStateChanged(pending_load.playlist_, this);
}

void SpotifyClient::PlaylistStateChanged(sp_playlist* pl, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  // If the playlist isn't loaded yet we have to wait
  if (!sp_playlist_is_loaded(pl)) {
    qLog(Debug) << "Playlist isn't loaded yet, waiting";
    return;
  }

  // Find this playlist's pending load object
  int pending_load_index = -1;
  PendingLoadPlaylist* pending_load = NULL;
  for (int i=0 ; i<me->pending_load_playlists_.count() ; ++i) {
    if (me->pending_load_playlists_[i].playlist_ == pl) {
      pending_load_index = i;
      pending_load = &me->pending_load_playlists_[i];
      break;
    }
  }

  if (!pending_load) {
    qLog(Warning) << "Playlist not found in pending load list";
    return;
  }

  // If the playlist was just loaded then get all its tracks and ref them
  if (pending_load->tracks_.isEmpty()) {
    const int count = sp_playlist_num_tracks(pl);
    for (int i=0 ; i<count ; ++i) {
      sp_track* track = sp_playlist_track(pl, i);
      sp_track_add_ref(track);
      pending_load->tracks_ << track;
    }
  }

  // If any of the tracks aren't loaded yet we have to wait
  foreach (sp_track* track, pending_load->tracks_) {
    if (!sp_track_is_loaded(track)) {
      qLog(Debug) << "One or more tracks aren't loaded yet, waiting";
      return;
    }
  }

  // Everything is loaded so send the response protobuf and unref everything.
  protobuf::SpotifyMessage message;
  protobuf::LoadPlaylistResponse* response = message.mutable_load_playlist_response();

  *response->mutable_request() = pending_load->request_;
  foreach (sp_track* track, pending_load->tracks_) {
    me->ConvertTrack(track, response->add_track());
    sp_track_release(track);
  }
  me->SendMessage(me->socket_, message);

  // Unref the playlist and remove our callbacks
  sp_playlist_remove_callbacks(pl, &me->load_playlist_callbacks_, me);
  sp_playlist_release(pl);

  // Remove the pending load object
  me->pending_load_playlists_.removeAt(pending_load_index);
}

void SpotifyClient::ConvertTrack(sp_track* track, protobuf::Track* pb) {
  sp_album* album = sp_track_album(track);

  pb->set_starred(sp_track_is_starred(session_, track));
  pb->set_title(sp_track_name(track));
  pb->set_album(sp_album_name(album));
  pb->set_year(sp_album_year(album));
  pb->set_duration_msec(sp_track_duration(track));
  pb->set_popularity(sp_track_popularity(track));
  pb->set_disc(sp_track_disc(track));
  pb->set_track(sp_track_index(track));

  for (int i=0 ; i<sp_track_num_artists(track) ; ++i) {
    pb->add_artist(sp_artist_name(sp_track_artist(track, i)));
  }

  // Blugh
  char uri[256];
  sp_link* link = sp_link_create_from_track(track, 0);
  sp_link_as_string(link, uri, sizeof(uri));
  sp_link_release(link);

  pb->set_uri(uri);
}

void SpotifyClient::MetadataUpdatedCallback(sp_session* session) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));

  foreach (const PendingLoadPlaylist& load, me->pending_load_playlists_) {
    PlaylistStateChanged(load.playlist_, me);
  }
}
