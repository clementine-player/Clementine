/* This file is part of Clementine.
   Copyright 2011-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011-2012, 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, pie.or.paj <pie.or.paj@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_SPOTIFY_SPOTIFYSERVER_H_
#define INTERNET_SPOTIFY_SPOTIFYSERVER_H_

#include <QImage>
#include <QObject>

#include "core/messagehandler.h"
#include "spotifymessages.pb.h"

class QTcpServer;
class QTcpSocket;

class SpotifyServer : public AbstractMessageHandler<cpb::spotify::Message> {
  Q_OBJECT

 public:
  explicit SpotifyServer(QObject* parent = nullptr);

  void Init();
  void Login(const QString& username, const QString& password,
             cpb::spotify::Bitrate bitrate, bool volume_normalisation);

  void LoadStarred();
  void SyncStarred();
  void LoadInbox();
  void SyncInbox();
  void LoadUserPlaylist(int index);
  void SyncUserPlaylist(int index);
  void AddSongsToStarred(const QList<QUrl>& songs_urls);
  void AddSongsToUserPlaylist(int playlist_index,
                              const QList<QUrl>& songs_urls);
  void RemoveSongsFromUserPlaylist(int playlist_index,
                                   const QList<int>& songs_indices_to_remove);
  void RemoveSongsFromStarred(const QList<int>& songs_indices_to_remove);
  void Search(const QString& text, int limit, int limit_album = 0);
  void LoadImage(const QString& id);
  void AlbumBrowse(const QString& uri);
  void SetPlaybackSettings(cpb::spotify::Bitrate bitrate,
                           bool volume_normalisation);
  void LoadToplist();
  void SetPaused(const bool paused);

  int server_port() const;

 public slots:
  void StartPlayback(const QString& uri, quint16 port);
  void Seek(qint64 offset_nsec);

 signals:
  void LoginCompleted(bool success, const QString& error,
                      cpb::spotify::LoginResponse_Error error_code);
  void PlaylistsUpdated(const cpb::spotify::Playlists& playlists);

  void StarredLoaded(const cpb::spotify::LoadPlaylistResponse& response);
  void InboxLoaded(const cpb::spotify::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const cpb::spotify::LoadPlaylistResponse& response);
  void PlaybackError(const QString& message);
  void SearchResults(const cpb::spotify::SearchResponse& response);
  void ImageLoaded(const QString& id, const QImage& image);
  void SyncPlaylistProgress(const cpb::spotify::SyncPlaylistProgress& progress);
  void AlbumBrowseResults(const cpb::spotify::BrowseAlbumResponse& response);
  void ToplistBrowseResults(
      const cpb::spotify::BrowseToplistResponse& response);

 protected:
  void MessageArrived(const cpb::spotify::Message& message);

 private slots:
  void NewConnection();

 private:
  void LoadPlaylist(cpb::spotify::PlaylistType type, int index = -1);
  void SyncPlaylist(cpb::spotify::PlaylistType type, int index, bool offline);
  void AddSongsToPlaylist(const cpb::spotify::PlaylistType playlist_type,
                          const QList<QUrl>& songs_urls,
                          // Used iff type is user_playlist
                          int playlist_index = -1);
  void RemoveSongsFromPlaylist(const cpb::spotify::PlaylistType playlist_type,
                               const QList<int>& songs_indices_to_remove,
                               // Used iff type is user_playlist
                               int playlist_index = -1);
  void SendOrQueueMessage(const cpb::spotify::Message& message);

  QTcpServer* server_;
  bool logged_in_;

  QList<cpb::spotify::Message> queued_login_messages_;
  QList<cpb::spotify::Message> queued_messages_;
};

#endif  // INTERNET_SPOTIFY_SPOTIFYSERVER_H_
