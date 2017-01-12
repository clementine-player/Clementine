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

#include <algorithm>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

#include "core/arraysize.h"
#include "core/logging.h"
#include "core/timeconstants.h"
#include "mediapipeline.h"
#include "spotifykey.h"
#include "spotifymessages.pb.h"
#include "spotify_utilities.h"

const int SpotifyClient::kSpotifyImageIDSize = 20;
const int SpotifyClient::kWaveHeaderSize = 44;

SpotifyClient::SpotifyClient(QObject* parent)
    : AbstractMessageHandler<pb::spotify::Message>(nullptr, parent),
      api_key_(QByteArray::fromBase64(kSpotifyApiKey)),
      protocol_socket_(new QTcpSocket(this)),
      session_(nullptr),
      events_timer_(new QTimer(this)) {
  SetDevice(protocol_socket_);

  memset(&spotify_callbacks_, 0, sizeof(spotify_callbacks_));
  memset(&spotify_config_, 0, sizeof(spotify_config_));
  memset(&playlistcontainer_callbacks_, 0,
         sizeof(playlistcontainer_callbacks_));
  memset(&get_playlists_callbacks_, 0, sizeof(get_playlists_callbacks_));
  memset(&load_playlist_callbacks_, 0, sizeof(load_playlist_callbacks_));

  spotify_callbacks_.logged_in = &LoggedInCallback;
  spotify_callbacks_.notify_main_thread = &NotifyMainThreadCallback;
  spotify_callbacks_.log_message = &LogMessageCallback;
  spotify_callbacks_.metadata_updated = &MetadataUpdatedCallback;
  spotify_callbacks_.music_delivery = &MusicDeliveryCallback;
  spotify_callbacks_.end_of_track = &EndOfTrackCallback;
  spotify_callbacks_.streaming_error = &StreamingErrorCallback;
  spotify_callbacks_.offline_status_updated = &OfflineStatusUpdatedCallback;
  spotify_callbacks_.connection_error = &ConnectionErrorCallback;
  spotify_callbacks_.message_to_user = &UserMessageCallback;
  spotify_callbacks_.start_playback = &StartPlaybackCallback;
  spotify_callbacks_.stop_playback = &StopPlaybackCallback;

  playlistcontainer_callbacks_.container_loaded =
      &PlaylistContainerLoadedCallback;
  playlistcontainer_callbacks_.playlist_added = &PlaylistAddedCallback;
  playlistcontainer_callbacks_.playlist_moved = &PlaylistMovedCallback;
  playlistcontainer_callbacks_.playlist_removed = &PlaylistRemovedCallback;

  get_playlists_callbacks_.playlist_state_changed =
      &PlaylistStateChangedForGetPlaylists;

  load_playlist_callbacks_.playlist_state_changed =
      &PlaylistStateChangedForLoadPlaylist;

  QString cache = utilities::GetCacheDirectory();
  qLog(Debug) << "Using:" << cache << "for Spotify cache";
  QString settings_dir = utilities::GetSettingsDirectory();
  qLog(Debug) << "Using:" << settings_dir << "for Spotify settings";

  spotify_config_.api_version = SPOTIFY_API_VERSION;  // From libspotify/api.h
  spotify_config_.cache_location = strdup(cache.toUtf8().constData());
  spotify_config_.settings_location = strdup(settings_dir.toUtf8().constData());
  spotify_config_.application_key = api_key_.constData();
  spotify_config_.application_key_size = api_key_.size();
  spotify_config_.callbacks = &spotify_callbacks_;
  spotify_config_.userdata = this;
  spotify_config_.user_agent = "Clementine Player";

  events_timer_->setSingleShot(true);
  connect(events_timer_, SIGNAL(timeout()), SLOT(ProcessEvents()));

  connect(protocol_socket_, SIGNAL(disconnected()),
          QCoreApplication::instance(), SLOT(quit()));
}

SpotifyClient::~SpotifyClient() {
  if (session_) {
    sp_session_release(session_);
  }

  free(const_cast<char*>(spotify_config_.cache_location));
  free(const_cast<char*>(spotify_config_.settings_location));
}

void SpotifyClient::Init(quint16 port) {
  qLog(Debug) << "Connecting to port" << port;

  protocol_socket_->connectToHost(QHostAddress::LocalHost, port);
}

void SpotifyClient::LoggedInCallback(sp_session* session, sp_error error) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));
  const bool success = error == SP_ERROR_OK;
  pb::spotify::LoginResponse_Error error_code =
      pb::spotify::LoginResponse_Error_Other;

  if (!success) {
    qLog(Warning) << "Failed to login" << sp_error_message(error);
  }

  switch (error) {
    case SP_ERROR_BAD_USERNAME_OR_PASSWORD:
      error_code = pb::spotify::LoginResponse_Error_BadUsernameOrPassword;
      break;
    case SP_ERROR_USER_BANNED:
      error_code = pb::spotify::LoginResponse_Error_UserBanned;
      break;
    case SP_ERROR_USER_NEEDS_PREMIUM:
      error_code = pb::spotify::LoginResponse_Error_UserNeedsPremium;
      break;
    default:
      error_code = pb::spotify::LoginResponse_Error_Other;
      break;
  }

  me->SendLoginCompleted(success, sp_error_message(error), error_code);

  if (success) {
    sp_playlistcontainer_add_callbacks(sp_session_playlistcontainer(session),
                                       &me->playlistcontainer_callbacks_, me);
    sp_session_flush_caches(me->session_);
  }
}

void SpotifyClient::NotifyMainThreadCallback(sp_session* session) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));
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

void SpotifyClient::Search(const pb::spotify::SearchRequest& req) {
  sp_search* search =
      sp_search_create(session_, req.query().c_str(), 0, req.limit(), 0,
                       req.limit_album(), 0, 0,  // artists
                       0, 0,                     // playlists
                       SP_SEARCH_STANDARD, &SearchCompleteCallback, this);

  pending_searches_[search] = req;
}

void SpotifyClient::SearchCompleteCallback(sp_search* result, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  if (!me->pending_searches_.contains(result)) {
    qLog(Warning) << "SearchComplete called with unknown search";
    return;
  }

  // If there were any album results then we need to resolve those before
  // we can send our response.
  const int count = sp_search_num_albums(result);
  if (count != 0) {
    for (int i = 0; i < count; ++i) {
      sp_album* album = sp_search_album(result, i);
      sp_albumbrowse* browse = sp_albumbrowse_create(
          me->session_, album, &SearchAlbumBrowseComplete, me);

      me->pending_search_album_browse_responses_[browse] = result;
    }
    return;
  }

  me->SendSearchResponse(result);
}

void SpotifyClient::SearchAlbumBrowseComplete(sp_albumbrowse* result,
                                              void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  if (!me->pending_search_album_browse_responses_.contains(result)) {
    qLog(Warning) << "SearchAlbumBrowseComplete called with unknown result";
    return;
  }

  sp_search* search = me->pending_search_album_browse_responses_.take(result);
  me->pending_search_album_browses_[search].append(result);

  if (me->pending_search_album_browses_[search].count() >=
      sp_search_num_albums(search)) {
    me->SendSearchResponse(search);
  }
}

void SpotifyClient::SendSearchResponse(sp_search* result) {
  // Take the request out of the queue
  pb::spotify::SearchRequest req = pending_searches_.take(result);

  // Prepare the response
  pb::spotify::Message message;
  pb::spotify::SearchResponse* response = message.mutable_search_response();

  *response->mutable_request() = req;

  // Check for errors
  sp_error error = sp_search_error(result);
  if (error != SP_ERROR_OK) {
    response->set_error(sp_error_message(error));

    SendMessage(message);
    sp_search_release(result);
    return;
  }

  // Get the list of tracks from the search
  int count = sp_search_num_tracks(result);
  for (int i = 0; i < count; ++i) {
    sp_track* track = sp_search_track(result, i);
    ConvertTrack(track, response->add_result());
  }

  // Get the albums from the search.  All these should be resolved by now.
  QList<sp_albumbrowse*> browses = pending_search_album_browses_.take(result);
  for (sp_albumbrowse* browse : browses) {
    sp_album* album = sp_albumbrowse_album(browse);
    pb::spotify::Album* msg = response->add_album();

    ConvertAlbum(album, msg->mutable_metadata());
    ConvertAlbumBrowse(browse, msg->mutable_metadata());

    // Add all tracks
    const int tracks = sp_albumbrowse_num_tracks(browse);
    for (int i = 0; i < tracks; ++i) {
      ConvertTrack(sp_albumbrowse_track(browse, i), msg->add_track());
    }

    sp_albumbrowse_release(browse);
  }

  // Add other data to the response
  response->set_total_tracks(sp_search_total_tracks(result));
  response->set_did_you_mean(sp_search_did_you_mean(result));

  SendMessage(message);
  sp_search_release(result);
}

void SpotifyClient::MessageArrived(const pb::spotify::Message& message) {
  if (message.has_login_request()) {
    Login(message.login_request());
  } else if (message.has_load_playlist_request()) {
    LoadPlaylist(message.load_playlist_request());
  } else if (message.has_playback_request()) {
    StartPlayback(message.playback_request());
  } else if (message.has_seek_request()) {
    Seek(message.seek_request().offset_nsec());
  } else if (message.has_search_request()) {
    Search(message.search_request());
  } else if (message.has_image_request()) {
    LoadImage(QStringFromStdString(message.image_request().id()));
  } else if (message.has_sync_playlist_request()) {
    SyncPlaylist(message.sync_playlist_request());
  } else if (message.has_browse_album_request()) {
    BrowseAlbum(QStringFromStdString(message.browse_album_request().uri()));
  } else if (message.has_set_playback_settings_request()) {
    SetPlaybackSettings(message.set_playback_settings_request());
  } else if (message.has_browse_toplist_request()) {
    BrowseToplist(message.browse_toplist_request());
  } else if (message.has_pause_request()) {
    SetPaused(message.pause_request());
  } else if (message.has_add_tracks_to_playlist()) {
    AddTracksToPlaylist(message.add_tracks_to_playlist());
  } else if (message.has_remove_tracks_from_playlist()) {
    RemoveTracksFromPlaylist(message.remove_tracks_from_playlist());
  }
}

void SpotifyClient::SetPlaybackSettings(
    const pb::spotify::PlaybackSettings& req) {
  sp_bitrate bitrate = SP_BITRATE_320k;
  switch (req.bitrate()) {
    case pb::spotify::Bitrate96k:
      bitrate = SP_BITRATE_96k;
      break;
    case pb::spotify::Bitrate160k:
      bitrate = SP_BITRATE_160k;
      break;
    case pb::spotify::Bitrate320k:
      bitrate = SP_BITRATE_320k;
      break;
  }

  qLog(Debug) << "Setting playback settings: bitrate" << bitrate
              << "normalisation" << req.volume_normalisation();

  sp_session_preferred_bitrate(session_, bitrate);
  sp_session_preferred_offline_bitrate(session_, bitrate, false);
  sp_session_set_volume_normalization(session_, req.volume_normalisation());
}

void SpotifyClient::Login(const pb::spotify::LoginRequest& req) {
  sp_error error = sp_session_create(&spotify_config_, &session_);
  if (error != SP_ERROR_OK) {
    qLog(Warning) << "Failed to create session" << sp_error_message(error);
    SendLoginCompleted(false, sp_error_message(error),
                       pb::spotify::LoginResponse_Error_Other);
    return;
  }

  SetPlaybackSettings(req.playback_settings());

  if (req.password().empty()) {
    sp_error error = sp_session_relogin(session_);
    if (error != SP_ERROR_OK) {
      qLog(Warning) << "Tried to relogin but no stored credentials";
      SendLoginCompleted(false, sp_error_message(error),
                         pb::spotify::LoginResponse_Error_ReloginFailed);
    }
  } else {
    sp_session_login(session_, req.username().c_str(), req.password().c_str(),
                     true,  // Remember the password.
                     nullptr);
  }
}

void SpotifyClient::SendLoginCompleted(
    bool success, const QString& error,
    pb::spotify::LoginResponse_Error error_code) {
  pb::spotify::Message message;

  pb::spotify::LoginResponse* response = message.mutable_login_response();
  response->set_success(success);
  response->set_error(DataCommaSizeFromQString(error));

  if (!success) {
    response->set_error_code(error_code);
  }

  SendMessage(message);
}

void SpotifyClient::PlaylistContainerLoadedCallback(sp_playlistcontainer* pc,
                                                    void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  // Install callbacks on all the playlists
  sp_playlist_add_callbacks(sp_session_starred_create(me->session_),
                            &me->get_playlists_callbacks_, me);
  const int count = sp_playlistcontainer_num_playlists(pc);
  for (int i = 0; i < count; ++i) {
    sp_playlist* playlist = sp_playlistcontainer_playlist(pc, i);
    sp_playlist_add_callbacks(playlist, &me->get_playlists_callbacks_, me);
  }

  me->SendPlaylistList();
}

void SpotifyClient::PlaylistAddedCallback(sp_playlistcontainer* pc,
                                          sp_playlist* playlist, int position,
                                          void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  // Install callbacks on this playlist
  sp_playlist_add_callbacks(playlist, &me->get_playlists_callbacks_, me);

  me->SendPlaylistList();
}

void SpotifyClient::PlaylistMovedCallback(sp_playlistcontainer* pc,
                                          sp_playlist* playlist, int position,
                                          int new_position, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);
  me->SendPlaylistList();
}

void SpotifyClient::PlaylistRemovedCallback(sp_playlistcontainer* pc,
                                            sp_playlist* playlist, int position,
                                            void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  // Remove callbacks from this playlist
  sp_playlist_remove_callbacks(playlist, &me->get_playlists_callbacks_, me);

  me->SendPlaylistList();
}

void SpotifyClient::SendPlaylistList() {
  pb::spotify::Message message;
  pb::spotify::Playlists* response = message.mutable_playlists_updated();

  sp_playlistcontainer* container = sp_session_playlistcontainer(session_);
  if (!container) {
    qLog(Warning) << "sp_session_playlistcontainer returned nullptr";
    return;
  }

  const int count = sp_playlistcontainer_num_playlists(container);

  for (int i = 0; i < count; ++i) {
    const int type = sp_playlistcontainer_playlist_type(container, i);
    sp_playlist* playlist = sp_playlistcontainer_playlist(container, i);
    const bool is_loaded = sp_playlist_is_loaded(playlist);

    qLog(Debug) << "Got playlist" << i << is_loaded << type
                << sp_playlist_name(playlist);

    if (!is_loaded) {
      qLog(Info) << "Playlist is not loaded yet, jump to the next one...";
      continue;
    }

    if (type != SP_PLAYLIST_TYPE_PLAYLIST) {
      // Just ignore folders for now
      continue;
    }

    pb::spotify::Playlists::Playlist* msg = response->add_playlist();
    msg->set_index(i);
    msg->set_name(sp_playlist_name(playlist));
    sp_user* playlist_owner = sp_playlist_owner(playlist);
    msg->set_is_mine(sp_session_user(session_) == playlist_owner);
    msg->set_owner(sp_user_display_name(playlist_owner));

    sp_playlist_offline_status offline_status =
        sp_playlist_get_offline_status(session_, playlist);
    const bool is_offline = offline_status == SP_PLAYLIST_OFFLINE_STATUS_YES;
    msg->set_is_offline(is_offline);
    if (offline_status == SP_PLAYLIST_OFFLINE_STATUS_DOWNLOADING) {
      msg->set_download_progress(
          sp_playlist_get_offline_download_completed(session_, playlist));
    } else if (offline_status == SP_PLAYLIST_OFFLINE_STATUS_WAITING) {
      msg->set_download_progress(0);
    }
    msg->set_nb_tracks(sp_playlist_num_tracks(playlist));
    // URI - Blugh
    char uri[256];
    sp_link* link = sp_link_create_from_playlist(playlist);
    sp_link_as_string(link, uri, arraysize(uri));
    sp_link_release(link);
    msg->set_uri(uri);
  }

  SendMessage(message);
}

sp_playlist* SpotifyClient::GetPlaylist(pb::spotify::PlaylistType type,
                                        int user_index) {
  sp_playlist* playlist = nullptr;
  switch (type) {
    case pb::spotify::Inbox:
      playlist = sp_session_inbox_create(session_);
      break;

    case pb::spotify::Starred:
      playlist = sp_session_starred_create(session_);
      break;

    case pb::spotify::UserPlaylist: {
      sp_playlistcontainer* pc = sp_session_playlistcontainer(session_);

      if (pc && user_index <= sp_playlistcontainer_num_playlists(pc)) {
        if (sp_playlistcontainer_playlist_type(pc, user_index) ==
            SP_PLAYLIST_TYPE_PLAYLIST) {
          playlist = sp_playlistcontainer_playlist(pc, user_index);
          sp_playlist_add_ref(playlist);
        }
      }

      break;
    }
  }
  return playlist;
}

void SpotifyClient::LoadPlaylist(const pb::spotify::LoadPlaylistRequest& req) {
  PendingLoadPlaylist pending_load;
  pending_load.request_ = req;
  pending_load.playlist_ = GetPlaylist(req.type(), req.user_playlist_index());

  // A null playlist might mean the user wasn't logged in, or an invalid
  // playlist index was requested, so we'd better return an error straight away.
  if (!pending_load.playlist_) {
    qLog(Warning) << "Invalid playlist requested or not logged in";

    pb::spotify::Message message;
    pb::spotify::LoadPlaylistResponse* response =
        message.mutable_load_playlist_response();
    *response->mutable_request() = req;
    SendMessage(message);
    return;
  }

  sp_playlist_add_callbacks(pending_load.playlist_, &load_playlist_callbacks_,
                            this);
  pending_load_playlists_ << pending_load;

  PlaylistStateChangedForLoadPlaylist(pending_load.playlist_, this);
}

void SpotifyClient::SyncPlaylist(const pb::spotify::SyncPlaylistRequest& req) {
  sp_playlist* playlist =
      GetPlaylist(req.request().type(), req.request().user_playlist_index());

  // The playlist should already be loaded.
  sp_playlist_set_offline_mode(session_, playlist, req.offline_sync());
}

void SpotifyClient::PlaylistStateChangedForLoadPlaylist(sp_playlist* pl,
                                                        void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  // If the playlist isn't loaded yet we have to wait
  if (!sp_playlist_is_loaded(pl)) {
    qLog(Debug) << "Playlist isn't loaded yet, waiting";
    return;
  }

  // Find this playlist's pending load object
  int pending_load_index = -1;
  PendingLoadPlaylist* pending_load = nullptr;
  for (int i = 0; i < me->pending_load_playlists_.count(); ++i) {
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
    for (int i = 0; i < count; ++i) {
      sp_track* track = sp_playlist_track(pl, i);
      sp_track_add_ref(track);
      pending_load->tracks_ << track;
    }
  }

  // If any of the tracks aren't loaded yet we have to wait
  for (sp_track* track : pending_load->tracks_) {
    if (!sp_track_is_loaded(track)) {
      qLog(Debug) << "One or more tracks aren't loaded yet, waiting";
      return;
    }
  }

  // Everything is loaded so send the response protobuf and unref everything.
  pb::spotify::Message message;
  pb::spotify::LoadPlaylistResponse* response =
      message.mutable_load_playlist_response();

  // For some reason, we receive the starred tracks in reverse order but not
  // other playlists.
  if (pending_load->request_.type() == pb::spotify::Starred) {
    std::reverse(pending_load->tracks_.begin(), pending_load->tracks_.end());
  }

  *response->mutable_request() = pending_load->request_;
  for (sp_track* track : pending_load->tracks_) {
    me->ConvertTrack(track, response->add_track());
    sp_track_release(track);
  }
  me->SendMessage(message);

  // Unref the playlist and remove our callbacks
  sp_playlist_remove_callbacks(pl, &me->load_playlist_callbacks_, me);
  sp_playlist_release(pl);

  // Remove the pending load object
  me->pending_load_playlists_.removeAt(pending_load_index);
}

void SpotifyClient::PlaylistStateChangedForGetPlaylists(sp_playlist* pl,
                                                        void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  me->SendPlaylistList();
}

void SpotifyClient::AddTracksToPlaylist(
    const pb::spotify::AddTracksToPlaylistRequest& req) {
  // Get the playlist we want to update
  sp_playlist* playlist =
      GetPlaylist(req.playlist_type(), req.playlist_index());
  if (!playlist) {
    qLog(Error) << "Playlist " << req.playlist_type() << ","
                << req.playlist_index() << "not found";
    return;
  }

  // Get the tracks we want to add
  std::unique_ptr<sp_track* []> tracks_array(
      new sp_track* [req.track_uri_size()]);
  for (int i = 0; i < req.track_uri_size(); ++i) {
    sp_link* track_link = sp_link_create_from_string(req.track_uri(i).c_str());
    sp_track* track = sp_link_as_track(track_link);
    sp_track_add_ref(track);
    sp_link_release(track_link);
    if (!track) {
      qLog(Error) << "Track" << QString::fromStdString(req.track_uri(i))
                  << "not found";
    }
    tracks_array[i] = track;
  }

  // Actually add the tracks to the playlist
  if (sp_playlist_add_tracks(playlist, tracks_array.get(), req.track_uri_size(),
                             0 /* TODO: don't insert at a hardcoded position */,
                             session_) != SP_ERROR_OK) {
    qLog(Error) << "Error when adding tracks!";
  }

  // Clean everything
  for (int i = 0; i < req.track_uri_size(); ++i) {
    sp_track_release(tracks_array[i]);
  }
}

void SpotifyClient::RemoveTracksFromPlaylist(
    const pb::spotify::RemoveTracksFromPlaylistRequest& req) {
  // Get the playlist we want to update
  sp_playlist* playlist =
      GetPlaylist(req.playlist_type(), req.playlist_index());
  if (!playlist) {
    qLog(Error) << "Playlist " << req.playlist_type() << ","
                << req.playlist_index() << "not found";
    return;
  }

  // Get the position of the tracks we want to remove
  std::unique_ptr<int[]> tracks_indices_array(new int[req.track_index_size()]);
  for (int i = 0; i < req.track_index_size(); ++i) {
    tracks_indices_array[i] = req.track_index(i);
  }

  // WTF: sp_playlist_remove_tracks indexes start from the end for starred
  // playlist, not from the beginning like other playlists: reverse them
  if (req.playlist_type() == pb::spotify::Starred) {
    int num_tracks = sp_playlist_num_tracks(playlist);
    for (int i = 0; i < req.track_index_size(); i++) {
      tracks_indices_array[i] = num_tracks - tracks_indices_array[i] - 1;
    }
  }

  if (sp_playlist_remove_tracks(playlist, tracks_indices_array.get(),
                                req.track_index_size()) != SP_ERROR_OK) {
    qLog(Error) << "Error when removing tracks!";
  }
}

void SpotifyClient::ConvertTrack(sp_track* track, pb::spotify::Track* pb) {
  sp_album* album = sp_track_album(track);

  pb->set_starred(sp_track_is_starred(session_, track));
  pb->set_title(sp_track_name(track));
  pb->set_album(sp_album_name(album));
  pb->set_year(sp_album_year(album));
  pb->set_duration_msec(sp_track_duration(track));
  pb->set_popularity(sp_track_popularity(track));
  pb->set_disc(sp_track_disc(track));
  pb->set_track(sp_track_index(track));

  // Album art
  const QByteArray art_id(reinterpret_cast<const char*>(sp_album_cover(
                              sp_track_album(track), SP_IMAGE_SIZE_LARGE)),
                          kSpotifyImageIDSize);
  const QString art_id_b64 = QString::fromAscii(art_id.toBase64());
  pb->set_album_art_id(DataCommaSizeFromQString(art_id_b64));

  // Artists
  for (int i = 0; i < sp_track_num_artists(track); ++i) {
    pb->add_artist(sp_artist_name(sp_track_artist(track, i)));
  }

  // URI - Blugh
  char uri[256];
  sp_link* link = sp_link_create_from_track(track, 0);
  sp_link_as_string(link, uri, arraysize(uri));
  sp_link_release(link);

  pb->set_uri(uri);
}

void SpotifyClient::ConvertAlbum(sp_album* album, pb::spotify::Track* pb) {
  pb->set_album(sp_album_name(album));
  pb->set_year(sp_album_year(album));
  pb->add_artist(sp_artist_name(sp_album_artist(album)));

  // These fields were required in a previous version so need to set them again
  // now.
  pb->mutable_title();
  pb->set_duration_msec(-1);
  pb->set_popularity(-1);
  pb->set_disc(-1);
  pb->set_track(-1);
  pb->set_starred(false);

  // Album art
  const QByteArray art_id(
      reinterpret_cast<const char*>(sp_album_cover(album, SP_IMAGE_SIZE_LARGE)),
      kSpotifyImageIDSize);
  const QString art_id_b64 = QString::fromAscii(art_id.toBase64());
  pb->set_album_art_id(DataCommaSizeFromQString(art_id_b64));

  // URI - Blugh
  char uri[256];
  sp_link* link = sp_link_create_from_album(album);
  sp_link_as_string(link, uri, arraysize(uri));
  sp_link_release(link);

  pb->set_uri(uri);
}

void SpotifyClient::ConvertAlbumBrowse(sp_albumbrowse* browse,
                                       pb::spotify::Track* pb) {
  pb->set_track(sp_albumbrowse_num_tracks(browse));
}

void SpotifyClient::MetadataUpdatedCallback(sp_session* session) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));

  for (const PendingLoadPlaylist& load : me->pending_load_playlists_) {
    PlaylistStateChangedForLoadPlaylist(load.playlist_, me);
  }
  for (const PendingPlaybackRequest& playback :
       me->pending_playback_requests_) {
    me->TryPlaybackAgain(playback);
  }
}

int SpotifyClient::MusicDeliveryCallback(sp_session* session,
                                         const sp_audioformat* format,
                                         const void* frames, int num_frames) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));

  if (!me->media_pipeline_) {
    return 0;
  }

  if (num_frames == 0) {
    return 0;
  }

  if (!me->media_pipeline_->is_initialised()) {
    if (!me->media_pipeline_->Init(format->sample_rate, format->channels)) {
      qLog(Warning) << "Failed to intitialise media pipeline";
      sp_session_player_unload(me->session_);
      me->media_pipeline_.reset();
      return 0;
    }
  }

  if (!me->media_pipeline_->is_accepting_data()) {
    return 0;
  }

  me->media_pipeline_->WriteData(reinterpret_cast<const char*>(frames),
                                 num_frames * format->channels * 2);

  return num_frames;
}

void SpotifyClient::EndOfTrackCallback(sp_session* session) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));

  me->media_pipeline_.reset();
}

void SpotifyClient::StreamingErrorCallback(sp_session* session,
                                           sp_error error) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));

  me->media_pipeline_.reset();

  // Send the error
  me->SendPlaybackError(QString::fromUtf8(sp_error_message(error)));
}

void SpotifyClient::ConnectionErrorCallback(sp_session* session,
                                            sp_error error) {
  qLog(Debug) << Q_FUNC_INFO << sp_error_message(error);
}

void SpotifyClient::UserMessageCallback(sp_session* session,
                                        const char* message) {
  qLog(Debug) << Q_FUNC_INFO << message;
}

void SpotifyClient::StartPlaybackCallback(sp_session* session) {
  qLog(Debug) << Q_FUNC_INFO;
}

void SpotifyClient::StopPlaybackCallback(sp_session* session) {
  qLog(Debug) << Q_FUNC_INFO;
}

void SpotifyClient::OfflineStatusUpdatedCallback(sp_session* session) {
  SpotifyClient* me =
      reinterpret_cast<SpotifyClient*>(sp_session_userdata(session));
  sp_playlistcontainer* container = sp_session_playlistcontainer(session);
  if (!container) {
    qLog(Warning) << "sp_session_playlistcontainer returned nullptr";
    return;
  }

  const int count = sp_playlistcontainer_num_playlists(container);

  for (int i = 0; i < count; ++i) {
    const sp_playlist_type type =
        sp_playlistcontainer_playlist_type(container, i);
    sp_playlist* playlist = sp_playlistcontainer_playlist(container, i);

    if (type != SP_PLAYLIST_TYPE_PLAYLIST) {
      // Just ignore folders for now
      continue;
    }

    int download_progress = me->GetDownloadProgress(playlist);
    if (download_progress != -1) {
      me->SendDownloadProgress(pb::spotify::UserPlaylist, i, download_progress);
    }
  }

  sp_playlist* inbox = sp_session_inbox_create(session);
  int download_progress = me->GetDownloadProgress(inbox);
  sp_playlist_release(inbox);

  if (download_progress != -1) {
    me->SendDownloadProgress(pb::spotify::Inbox, -1, download_progress);
  }

  sp_playlist* starred = sp_session_starred_create(session);
  download_progress = me->GetDownloadProgress(starred);
  sp_playlist_release(starred);

  if (download_progress != -1) {
    me->SendDownloadProgress(pb::spotify::Starred, -1, download_progress);
  }
}

void SpotifyClient::SendDownloadProgress(pb::spotify::PlaylistType type,
                                         int index, int download_progress) {
  pb::spotify::Message message;
  pb::spotify::SyncPlaylistProgress* progress =
      message.mutable_sync_playlist_progress();
  progress->mutable_request()->set_type(type);
  if (index != -1) {
    progress->mutable_request()->set_user_playlist_index(index);
  }
  progress->set_sync_progress(download_progress);
  SendMessage(message);
}

int SpotifyClient::GetDownloadProgress(sp_playlist* playlist) {
  sp_playlist_offline_status status =
      sp_playlist_get_offline_status(session_, playlist);
  switch (status) {
    case SP_PLAYLIST_OFFLINE_STATUS_NO:
      return -1;
    case SP_PLAYLIST_OFFLINE_STATUS_YES:
      return 100;
    case SP_PLAYLIST_OFFLINE_STATUS_DOWNLOADING:
      return sp_playlist_get_offline_download_completed(session_, playlist);
    case SP_PLAYLIST_OFFLINE_STATUS_WAITING:
      return 0;
  }
  return -1;
}

void SpotifyClient::StartPlayback(const pb::spotify::PlaybackRequest& req) {
  // Get a link object from the URI
  sp_link* link = sp_link_create_from_string(req.track_uri().c_str());
  if (!link) {
    SendPlaybackError("Invalid Spotify URI");
    return;
  }

  // Get the track from the link
  sp_track* track = sp_link_as_track(link);
  if (!track) {
    SendPlaybackError("Spotify URI was not a track");
    sp_link_release(link);
    return;
  }

  PendingPlaybackRequest pending_playback;
  pending_playback.request_ = req;
  pending_playback.link_ = link;
  pending_playback.track_ = track;

  pending_playback_requests_ << pending_playback;

  TryPlaybackAgain(pending_playback);
}

void SpotifyClient::Seek(qint64 offset_nsec) {
  // TODO
  qLog(Error) << "TODO seeking";
}

void SpotifyClient::TryPlaybackAgain(const PendingPlaybackRequest& req) {
  // If the track was not loaded then we have to come back later
  if (!sp_track_is_loaded(req.track_)) {
    qLog(Debug) << "Playback track not loaded yet, will try again later";
    return;
  }

  // Load the track
  sp_error error = sp_session_player_load(session_, req.track_);
  if (error != SP_ERROR_OK) {
    SendPlaybackError("Spotify playback error: " +
                      QString::fromUtf8(sp_error_message(error)));
    sp_link_release(req.link_);

    // Remove this from the pending list now
    pending_playback_requests_.removeAll(req);
    return;
  }

  // Create the media socket
  media_pipeline_.reset(new MediaPipeline(req.request_.media_port(),
                                          sp_track_duration(req.track_)));

  qLog(Info) << "Starting playback of uri" << req.request_.track_uri().c_str()
             << "to port" << req.request_.media_port();

  // Start playback
  sp_session_player_play(session_, true);

  sp_link_release(req.link_);

  // Remove this from the pending list now
  pending_playback_requests_.removeAll(req);
}

void SpotifyClient::SendPlaybackError(const QString& error) {
  pb::spotify::Message message;
  pb::spotify::PlaybackError* msg = message.mutable_playback_error();

  msg->set_error(DataCommaSizeFromQString(error));
  SendMessage(message);
}

void SpotifyClient::LoadImage(const QString& id_b64) {
  QByteArray id = QByteArray::fromBase64(id_b64.toAscii());
  if (id.length() != kSpotifyImageIDSize) {
    qLog(Warning) << "Invalid image ID (did not decode to"
                  << kSpotifyImageIDSize << "bytes):" << id_b64;

    // Send an error response straight away
    pb::spotify::Message message;
    pb::spotify::ImageResponse* msg = message.mutable_image_response();
    msg->set_id(DataCommaSizeFromQString(id_b64));
    SendMessage(message);
    return;
  }

  PendingImageRequest pending_load;
  pending_load.id_ = id;
  pending_load.id_b64_ = id_b64;
  pending_load.image_ =
      sp_image_create(session_, reinterpret_cast<const byte*>(id.constData()));
  pending_image_requests_ << pending_load;

  if (!image_callbacks_registered_[pending_load.image_]) {
    sp_image_add_load_callback(pending_load.image_, &ImageLoaded, this);
  }
  image_callbacks_registered_[pending_load.image_]++;

  TryImageAgain(pending_load.image_);
}

void SpotifyClient::TryImageAgain(sp_image* image) {
  if (!sp_image_is_loaded(image)) {
    qLog(Debug) << "Image not loaded, will try again later";
    return;
  }

  // Find the pending request for this image
  int index = -1;
  PendingImageRequest* req = nullptr;
  for (int i = 0; i < pending_image_requests_.count(); ++i) {
    if (pending_image_requests_[i].image_ == image) {
      index = i;
      req = &pending_image_requests_[i];
      break;
    }
  }

  if (index == -1) {
    qLog(Warning) << "Image not found in pending load list";
    return;
  }

  // Get the image data
  size_t size = 0;
  const void* data = sp_image_data(image, &size);

  // Send the response
  pb::spotify::Message message;
  pb::spotify::ImageResponse* msg = message.mutable_image_response();
  msg->set_id(DataCommaSizeFromQString(req->id_b64_));
  if (data && size) {
    msg->set_data(data, size);
  }
  SendMessage(message);

  // Free stuff
  image_callbacks_registered_[image]--;

  // TODO: memory leak?
  // sp_image_remove_load_callback(image, &ImageLoaded, this);
  image_callbacks_registered_.remove(image);

  sp_image_release(image);
  pending_image_requests_.removeAt(index);
}

void SpotifyClient::ImageLoaded(sp_image* image, void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);
  me->TryImageAgain(image);
}

void SpotifyClient::BrowseAlbum(const QString& uri) {
  // Get a link object from the URI
  sp_link* link = sp_link_create_from_string(uri.toStdString().c_str());
  if (!link) {
    SendPlaybackError("Invalid Album URI");
    return;
  }

  // Get the album from the link
  sp_album* album = sp_link_as_album(link);
  if (!album) {
    SendPlaybackError("Spotify URI was not an album");
    sp_link_release(link);
    return;
  }

  sp_albumbrowse* browse =
      sp_albumbrowse_create(session_, album, &AlbumBrowseComplete, this);
  pending_album_browses_[browse] = uri;
}

void SpotifyClient::AlbumBrowseComplete(sp_albumbrowse* result,
                                        void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  if (!me->pending_album_browses_.contains(result)) return;

  QString uri = me->pending_album_browses_.take(result);

  pb::spotify::Message message;
  pb::spotify::BrowseAlbumResponse* msg =
      message.mutable_browse_album_response();

  msg->set_uri(DataCommaSizeFromQString(uri));

  const int count = sp_albumbrowse_num_tracks(result);
  for (int i = 0; i < count; ++i) {
    me->ConvertTrack(sp_albumbrowse_track(result, i), msg->add_track());
  }

  me->SendMessage(message);
  sp_albumbrowse_release(result);
}

void SpotifyClient::BrowseToplist(
    const pb::spotify::BrowseToplistRequest& req) {
  sp_toplistbrowse* browse = sp_toplistbrowse_create(
      session_, SP_TOPLIST_TYPE_TRACKS,  // TODO: Support albums and artists.
      SP_TOPLIST_REGION_EVERYWHERE,      // TODO: Support other regions.
      nullptr, &ToplistBrowseComplete, this);
  pending_toplist_browses_[browse] = req;
}

void SpotifyClient::SetPaused(const pb::spotify::PauseRequest& req) {
  sp_session_player_play(session_, !req.paused());
}

void SpotifyClient::ToplistBrowseComplete(sp_toplistbrowse* result,
                                          void* userdata) {
  SpotifyClient* me = reinterpret_cast<SpotifyClient*>(userdata);

  qLog(Debug) << "Toplist browse request took:"
              << sp_toplistbrowse_backend_request_duration(result) << "ms";

  if (!me->pending_toplist_browses_.contains(result)) {
    return;
  }

  const pb::spotify::BrowseToplistRequest& request =
      me->pending_toplist_browses_.take(result);

  pb::spotify::Message message;
  pb::spotify::BrowseToplistResponse* msg =
      message.mutable_browse_toplist_response();
  msg->mutable_request()->CopyFrom(request);

  const int count = sp_toplistbrowse_num_tracks(result);
  for (int i = 0; i < count; ++i) {
    me->ConvertTrack(sp_toplistbrowse_track(result, i), msg->add_track());
  }

  me->SendMessage(message);
  sp_toplistbrowse_release(result);
}

void SpotifyClient::DeviceClosed() {
  AbstractMessageHandler<pb::spotify::Message>::DeviceClosed();

  qApp->exit();
}
