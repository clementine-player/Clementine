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

#include "spotifyblob/common/spotifymessages.pb.h"

#include <QImage>
#include <QObject>

class SpotifyMessageHandler;

class QTcpServer;
class QTcpSocket;

class SpotifyServer : public QObject {
  Q_OBJECT

public:
  SpotifyServer(QObject* parent = 0);

  void Init();
  void Login(const QString& username, const QString& password);

  void LoadStarred();
  void SyncStarred();
  void LoadInbox();
  void SyncInbox();
  void LoadUserPlaylist(int index);
  void SyncUserPlaylist(int index);
  void StartPlayback(const QString& uri, quint16 port);
  void Search(const QString& text, int limit);
  void LoadImage(const QString& id);

  int server_port() const;

signals:
  void LoginCompleted(bool success, const QString& error,
                      protobuf::LoginResponse_Error error_code);
  void PlaylistsUpdated(const protobuf::Playlists& playlists);

  void StarredLoaded(const protobuf::LoadPlaylistResponse& response);
  void InboxLoaded(const protobuf::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const protobuf::LoadPlaylistResponse& response);
  void PlaybackError(const QString& message);
  void SearchResults(const protobuf::SearchResponse& response);
  void ImageLoaded(const QString& id, const QImage& image);
  void SyncPlaylistProgress(const protobuf::SyncPlaylistProgress& progress);

private slots:
  void NewConnection();
  void HandleMessage(const protobuf::SpotifyMessage& message);

private:
  void LoadPlaylist(protobuf::PlaylistType type, int index = -1);
  void SyncPlaylist(protobuf::PlaylistType type, int index, bool offline);
  void SendMessage(const protobuf::SpotifyMessage& message);

  QTcpServer* server_;
  QTcpSocket* protocol_socket_;
  SpotifyMessageHandler* handler_;
  bool logged_in_;

  QList<protobuf::SpotifyMessage> queued_login_messages_;
  QList<protobuf::SpotifyMessage> queued_messages_;
};

#endif // SPOTIFYSERVER_H
