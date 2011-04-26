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
#include "spotifymessageutils.h"

#include <QObject>

#include <libspotify/api.h>

class QTcpSocket;
class QTimer;

class ResponseMessage;

class SpotifyClient : public QObject, protected SpotifyMessageUtils {
  Q_OBJECT

public:
  SpotifyClient(QObject* parent = 0);
  ~SpotifyClient();

  void Init(quint16 port);

private slots:
  void SocketReadyRead();
  void ProcessEvents();
  void MediaSocketDisconnected();

private:
  void SendLoginCompleted(bool success, const QString& error);
  void SendPlaybackError(const QString& error);

  // Spotify session callbacks.
  static void LoggedInCallback(sp_session* session, sp_error error);
  static void NotifyMainThreadCallback(sp_session* session);
  static void LogMessageCallback(sp_session* session, const char* data);
  static void SearchCompleteCallback(sp_search* result, void* userdata);
  static void MetadataUpdatedCallback(sp_session* session);
  static int MusicDeliveryCallback(
    sp_session* session, const sp_audioformat* format,
    const void* frames, int num_frames);
  static void EndOfTrackCallback(sp_session* session);
  static void StreamingErrorCallback(sp_session* session, sp_error error);

  // Spotify playlist container callbacks.
  static void PlaylistAddedCallback(
    sp_playlistcontainer* pc, sp_playlist* playlist,
    int position, void* userdata);
  static void PlaylistRemovedCallback(
    sp_playlistcontainer* pc, sp_playlist* playlist,
    int position, void* userdata);
  static void PlaylistMovedCallback(
    sp_playlistcontainer* pc, sp_playlist* playlist,
    int position, int new_position, void* userdata);
  static void PlaylistContainerLoadedCallback(
    sp_playlistcontainer* pc, void* userdata);

  // Spotify playlist callbacks - when loading a playlist
  static void PlaylistStateChanged(sp_playlist* pl, void* userdata);

  // Request handlers.
  void Login(const QString& username, const QString& password);
  void GetPlaylists();
  void Search(const QString& query);
  void LoadPlaylist(const protobuf::LoadPlaylistRequest& req);
  void StartPlayback(const protobuf::PlaybackRequest& req);

  void ConvertTrack(sp_track* track, protobuf::Track* pb);

private:
  struct PendingLoadPlaylist {
    protobuf::LoadPlaylistRequest request_;

    sp_playlist* playlist_;

    QList<sp_track*> tracks_;
  };

  QByteArray api_key_;

  QTcpSocket* protocol_socket_;
  QTcpSocket* media_socket_;

  sp_session_config spotify_config_;
  sp_session_callbacks spotify_callbacks_;
  sp_playlistcontainer_callbacks playlistcontainer_callbacks_;
  sp_playlist_callbacks load_playlist_callbacks_;
  sp_session* session_;

  QTimer* events_timer_;

  QList<PendingLoadPlaylist> pending_load_playlists_;

  int media_length_msec_;
};

#endif // SPOTIFYCLIENT_H
