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

#include "spotifyserver.h"
#include "core/logging.h"

#include "spotifyblob/common/spotifymessages.pb.h"
#include "spotifyblob/common/spotifymessagehandler.h"

#include <QTcpServer>
#include <QTcpSocket>

SpotifyServer::SpotifyServer(QObject* parent)
  : QObject(parent),
    server_(new QTcpServer(this)),
    protocol_socket_(NULL),
    handler_(NULL),
    logged_in_(false)
{
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
}

void SpotifyServer::Init() {
  if (!server_->listen(QHostAddress::LocalHost)) {
    qLog(Error) << "Couldn't open server socket" << server_->errorString();
  }
}

int SpotifyServer::server_port() const {
  return server_->serverPort();
}

void SpotifyServer::NewConnection() {
  delete protocol_socket_;
  delete handler_;

  protocol_socket_ = server_->nextPendingConnection();
  handler_ = new SpotifyMessageHandler(protocol_socket_, this);
  connect(handler_, SIGNAL(MessageArrived(spotify_pb::SpotifyMessage)),
          SLOT(HandleMessage(spotify_pb::SpotifyMessage)));

  qLog(Info) << "Connection from port" << protocol_socket_->peerPort();

  // Send any login messages that were queued before the client connected
  foreach (const spotify_pb::SpotifyMessage& message, queued_login_messages_) {
    SendMessage(message);
  }
  queued_login_messages_.clear();
}

void SpotifyServer::SendMessage(const spotify_pb::SpotifyMessage& message) {
  const bool is_login_message = message.has_login_request();

  QList<spotify_pb::SpotifyMessage>* queue =
      is_login_message ? &queued_login_messages_ : &queued_messages_;

  if (!protocol_socket_ || (!is_login_message && !logged_in_)) {
    queue->append(message);
  } else {
    handler_->SendMessage(message);
  }
}

void SpotifyServer::Login(const QString& username, const QString& password) {
  spotify_pb::SpotifyMessage message;

  spotify_pb::LoginRequest* request = message.mutable_login_request();
  request->set_username(DataCommaSizeFromQString(username));
  request->set_password(DataCommaSizeFromQString(password));

  SendMessage(message);
}

void SpotifyServer::HandleMessage(const spotify_pb::SpotifyMessage& message) {
  if (message.has_login_response()) {
    const spotify_pb::LoginResponse& response = message.login_response();
    logged_in_ = response.success();

    if (response.success()) {
      // Send any messages that were queued before the client logged in
      foreach (const spotify_pb::SpotifyMessage& message, queued_messages_) {
        SendMessage(message);
      }
      queued_messages_.clear();
    }

    emit LoginCompleted(response.success(), QStringFromStdString(response.error()),
                        response.error_code());
  } else if (message.has_playlists_updated()) {
    emit PlaylistsUpdated(message.playlists_updated());
  } else if (message.has_load_playlist_response()) {
    const spotify_pb::LoadPlaylistResponse& response = message.load_playlist_response();

    switch (response.request().type()) {
      case spotify_pb::Inbox:
        emit InboxLoaded(response);
        break;

      case spotify_pb::Starred:
        emit StarredLoaded(response);
        break;

      case spotify_pb::UserPlaylist:
        emit UserPlaylistLoaded(response);
        break;
    }
  } else if (message.has_playback_error()) {
    emit PlaybackError(QStringFromStdString(message.playback_error().error()));
  } else if (message.has_search_response()) {
    emit SearchResults(message.search_response());
  } else if (message.has_image_response()) {
    const spotify_pb::ImageResponse& response = message.image_response();
    const QString id = QStringFromStdString(response.id());

    if (response.has_data()) {
      emit ImageLoaded(id, QImage::fromData(QByteArray(
          response.data().data(), response.data().size())));
    } else {
      emit ImageLoaded(id, QImage());
    }
  } else if (message.has_sync_playlist_progress()) {
    emit SyncPlaylistProgress(message.sync_playlist_progress());
  } else if (message.has_browse_album_response()) {
    emit AlbumBrowseResults(message.browse_album_response());
  }
}

void SpotifyServer::LoadPlaylist(spotify_pb::PlaylistType type, int index) {
  spotify_pb::SpotifyMessage message;
  spotify_pb::LoadPlaylistRequest* req = message.mutable_load_playlist_request();

  req->set_type(type);
  if (index != -1) {
    req->set_user_playlist_index(index);
  }

  SendMessage(message);
}

void SpotifyServer::SyncPlaylist(
    spotify_pb::PlaylistType type, int index, bool offline) {
  spotify_pb::SpotifyMessage message;
  spotify_pb::SyncPlaylistRequest* req = message.mutable_sync_playlist_request();
  req->mutable_request()->set_type(type);
  if (index != -1) {
    req->mutable_request()->set_user_playlist_index(index);
  }
  req->set_offline_sync(offline);

  SendMessage(message);
}

void SpotifyServer::SyncInbox() {
  SyncPlaylist(spotify_pb::Inbox, -1, true);
}

void SpotifyServer::SyncStarred() {
  SyncPlaylist(spotify_pb::Starred, -1, true);
}

void SpotifyServer::SyncUserPlaylist(int index) {
  Q_ASSERT(index >= 0);
  SyncPlaylist(spotify_pb::UserPlaylist, index, true);
}

void SpotifyServer::LoadInbox() {
  LoadPlaylist(spotify_pb::Inbox);
}

void SpotifyServer::LoadStarred() {
  LoadPlaylist(spotify_pb::Starred);
}

void SpotifyServer::LoadUserPlaylist(int index) {
  Q_ASSERT(index >= 0);
  LoadPlaylist(spotify_pb::UserPlaylist, index);
}

void SpotifyServer::StartPlayback(const QString& uri, quint16 port) {
  spotify_pb::SpotifyMessage message;
  spotify_pb::PlaybackRequest* req = message.mutable_playback_request();

  req->set_track_uri(DataCommaSizeFromQString(uri));
  req->set_media_port(port);
  SendMessage(message);
}

void SpotifyServer::Search(const QString& text, int limit, int limit_album) {
  spotify_pb::SpotifyMessage message;
  spotify_pb::SearchRequest* req = message.mutable_search_request();

  req->set_query(DataCommaSizeFromQString(text));
  req->set_limit(limit);
  req->set_limit_album(limit_album);
  SendMessage(message);
}

void SpotifyServer::LoadImage(const QString& id) {
  spotify_pb::SpotifyMessage message;
  spotify_pb::ImageRequest* req = message.mutable_image_request();

  req->set_id(DataCommaSizeFromQString(id));
  SendMessage(message);
}

void SpotifyServer::AlbumBrowse(const QString& uri) {
  spotify_pb::SpotifyMessage message;
  spotify_pb::BrowseAlbumRequest* req = message.mutable_browse_album_request();

  req->set_uri(DataCommaSizeFromQString(uri));
  SendMessage(message);
}
