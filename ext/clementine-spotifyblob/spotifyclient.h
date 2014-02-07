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

#ifndef SPOTIFYCLIENT_H
#define SPOTIFYCLIENT_H

#include "spotifymessages.pb.h"
#include "core/messagehandler.h"

#include <QMap>
#include <QObject>

#include <libspotify/api.h>

class QTcpSocket;
class QTimer;

class MediaPipeline;
class ResponseMessage;

class SpotifyClient : public AbstractMessageHandler<pb::spotify::Message> {
  Q_OBJECT

 public:
  SpotifyClient(QObject* parent = 0);
  ~SpotifyClient();

  static const int kSpotifyImageIDSize;
  static const int kWaveHeaderSize;

  void Init(quint16 port);

 protected:
  void MessageArrived(const pb::spotify::Message& message);
  void DeviceClosed();

 private slots:
  void ProcessEvents();

 private:
  void SendLoginCompleted(bool success, const QString& error,
                          pb::spotify::LoginResponse_Error error_code);
  void SendPlaybackError(const QString& error);
  void SendSearchResponse(sp_search* result);

  // Spotify session callbacks.
  static void SP_CALLCONV LoggedInCallback(sp_session* session, sp_error error);
  static void SP_CALLCONV NotifyMainThreadCallback(sp_session* session);
  static void SP_CALLCONV
      LogMessageCallback(sp_session* session, const char* data);
  static void SP_CALLCONV
      SearchCompleteCallback(sp_search* result, void* userdata);
  static void SP_CALLCONV MetadataUpdatedCallback(sp_session* session);
  static int SP_CALLCONV
      MusicDeliveryCallback(sp_session* session, const sp_audioformat* format,
                            const void* frames, int num_frames);
  static void SP_CALLCONV EndOfTrackCallback(sp_session* session);
  static void SP_CALLCONV
      StreamingErrorCallback(sp_session* session, sp_error error);
  static void SP_CALLCONV OfflineStatusUpdatedCallback(sp_session* session);
  static void SP_CALLCONV
      ConnectionErrorCallback(sp_session* session, sp_error error);
  static void SP_CALLCONV
      UserMessageCallback(sp_session* session, const char* message);
  static void SP_CALLCONV StartPlaybackCallback(sp_session* session);
  static void SP_CALLCONV StopPlaybackCallback(sp_session* session);

  // Spotify playlist container callbacks.
  static void SP_CALLCONV PlaylistAddedCallback(sp_playlistcontainer* pc,
                                                sp_playlist* playlist,
                                                int position, void* userdata);
  static void SP_CALLCONV PlaylistRemovedCallback(sp_playlistcontainer* pc,
                                                  sp_playlist* playlist,
                                                  int position, void* userdata);
  static void SP_CALLCONV
      PlaylistMovedCallback(sp_playlistcontainer* pc, sp_playlist* playlist,
                            int position, int new_position, void* userdata);
  static void SP_CALLCONV
      PlaylistContainerLoadedCallback(sp_playlistcontainer* pc, void* userdata);

  // Spotify playlist callbacks - when loading the list of playlists
  // initially
  static void SP_CALLCONV
      PlaylistStateChangedForGetPlaylists(sp_playlist* pl, void* userdata);

  // Spotify playlist callbacks - when loading a playlist
  static void SP_CALLCONV
      PlaylistStateChangedForLoadPlaylist(sp_playlist* pl, void* userdata);

  // Spotify image callbacks.
  static void SP_CALLCONV ImageLoaded(sp_image* image, void* userdata);

  // Spotify album browse callbacks.
  static void SP_CALLCONV
      SearchAlbumBrowseComplete(sp_albumbrowse* result, void* userdata);
  static void SP_CALLCONV
      AlbumBrowseComplete(sp_albumbrowse* result, void* userdata);

  // Spotify toplist browse callbacks.
  static void SP_CALLCONV
      ToplistBrowseComplete(sp_toplistbrowse* result, void* userdata);

  // Request handlers.
  void Login(const pb::spotify::LoginRequest& req);
  void Search(const pb::spotify::SearchRequest& req);
  void LoadPlaylist(const pb::spotify::LoadPlaylistRequest& req);
  void SyncPlaylist(const pb::spotify::SyncPlaylistRequest& req);
  void StartPlayback(const pb::spotify::PlaybackRequest& req);
  void Seek(qint64 offset_bytes);
  void LoadImage(const QString& id_b64);
  void BrowseAlbum(const QString& uri);
  void BrowseToplist(const pb::spotify::BrowseToplistRequest& req);
  void SetPlaybackSettings(const pb::spotify::PlaybackSettings& req);

  void SendPlaylistList();

  void ConvertTrack(sp_track* track, pb::spotify::Track* pb);
  void ConvertAlbum(sp_album* album, pb::spotify::Track* pb);
  void ConvertAlbumBrowse(sp_albumbrowse* browse, pb::spotify::Track* pb);

  // Gets the appropriate sp_playlist* but does not load it.
  sp_playlist* GetPlaylist(pb::spotify::PlaylistType type, int user_index);

 private:
  struct PendingLoadPlaylist {
    pb::spotify::LoadPlaylistRequest request_;
    sp_playlist* playlist_;
    QList<sp_track*> tracks_;
    bool offline_sync;
  };

  struct PendingPlaybackRequest {
    pb::spotify::PlaybackRequest request_;
    sp_link* link_;
    sp_track* track_;

    bool operator==(const PendingPlaybackRequest& other) const {
      return request_.track_uri() == other.request_.track_uri() &&
             request_.media_port() == other.request_.media_port();
    }
  };

  struct PendingImageRequest {
    QString id_b64_;
    QByteArray id_;
    sp_image* image_;
  };

  void TryPlaybackAgain(const PendingPlaybackRequest& req);
  void TryImageAgain(sp_image* image);
  int GetDownloadProgress(sp_playlist* playlist);
  void SendDownloadProgress(pb::spotify::PlaylistType type, int index,
                            int download_progress);

  QByteArray api_key_;

  QTcpSocket* protocol_socket_;

  sp_session_config spotify_config_;
  sp_session_callbacks spotify_callbacks_;
  sp_playlistcontainer_callbacks playlistcontainer_callbacks_;
  sp_playlist_callbacks get_playlists_callbacks_;
  sp_playlist_callbacks load_playlist_callbacks_;
  sp_session* session_;

  QTimer* events_timer_;

  QList<PendingLoadPlaylist> pending_load_playlists_;
  QList<PendingPlaybackRequest> pending_playback_requests_;
  QList<PendingImageRequest> pending_image_requests_;
  QMap<sp_image*, int> image_callbacks_registered_;
  QMap<sp_search*, pb::spotify::SearchRequest> pending_searches_;
  QMap<sp_albumbrowse*, QString> pending_album_browses_;
  QMap<sp_toplistbrowse*, pb::spotify::BrowseToplistRequest>
      pending_toplist_browses_;

  QMap<sp_search*, QList<sp_albumbrowse*> > pending_search_album_browses_;
  QMap<sp_albumbrowse*, sp_search*> pending_search_album_browse_responses_;

  QScopedPointer<MediaPipeline> media_pipeline_;
};

#endif  // SPOTIFYCLIENT_H
