/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef SPOTIFYSERVER_H
#define SPOTIFYSERVER_H

#include "spotifymessages.pb.h"
#include "core/messagehandler.h"

#include <QImage>
#include <QObject>

class QTcpServer;
class QTcpSocket;

class SpotifyServer : public AbstractMessageHandler<pb::spotify::Message> {
  Q_OBJECT

 public:
  SpotifyServer(QObject* parent = 0);

  void Init();
  void Login(const QString& username, const QString& password,
             pb::spotify::Bitrate bitrate, bool volume_normalisation);

  void LoadStarred();
  void SyncStarred();
  void LoadInbox();
  void SyncInbox();
  void LoadUserPlaylist(int index);
  void SyncUserPlaylist(int index);
  void StartPlaybackLater(const QString& uri, quint16 port);
  void Search(const QString& text, int limit, int limit_album = 0);
  void LoadImage(const QString& id);
  void AlbumBrowse(const QString& uri);
  void SetPlaybackSettings(pb::spotify::Bitrate bitrate,
                           bool volume_normalisation);
  void LoadToplist();

  int server_port() const;

 public slots:
  void StartPlayback(const QString& uri, quint16 port);
  void Seek(qint64 offset_bytes);

signals:
  void LoginCompleted(bool success, const QString& error,
                      pb::spotify::LoginResponse_Error error_code);
  void PlaylistsUpdated(const pb::spotify::Playlists& playlists);

  void StarredLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void InboxLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void PlaybackError(const QString& message);
  void SearchResults(const pb::spotify::SearchResponse& response);
  void ImageLoaded(const QString& id, const QImage& image);
  void SyncPlaylistProgress(const pb::spotify::SyncPlaylistProgress& progress);
  void AlbumBrowseResults(const pb::spotify::BrowseAlbumResponse& response);
  void ToplistBrowseResults(const pb::spotify::BrowseToplistResponse& response);

 protected:
  void MessageArrived(const pb::spotify::Message& message);

 private slots:
  void NewConnection();

 private:
  void LoadPlaylist(pb::spotify::PlaylistType type, int index = -1);
  void SyncPlaylist(pb::spotify::PlaylistType type, int index, bool offline);
  void SendOrQueueMessage(const pb::spotify::Message& message);

  QTcpServer* server_;
  bool logged_in_;

  QList<pb::spotify::Message> queued_login_messages_;
  QList<pb::spotify::Message> queued_messages_;
};

#endif  // SPOTIFYSERVER_H
