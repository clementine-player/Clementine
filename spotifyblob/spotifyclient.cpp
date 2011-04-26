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

  spotify_callbacks_.logged_in = &LoggedInCallback;
  spotify_callbacks_.notify_main_thread = &NotifyMainThreadCallback;
  spotify_callbacks_.log_message = &LogMessageCallback;

  playlistcontainer_callbacks_.container_loaded = &PlaylistContainerLoadedCallback;
  playlistcontainer_callbacks_.playlist_added = &PlaylistAddedCallback;
  playlistcontainer_callbacks_.playlist_moved = &PlaylistMovedCallback;
  playlistcontainer_callbacks_.playlist_removed = &PlaylistRemovedCallback;

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
  qLog(Debug) << "connecting to port" << port;

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
  qLog(Debug) << "libspotify:" << data;
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
