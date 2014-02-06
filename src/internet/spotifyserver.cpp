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
#include "core/closure.h"
#include "core/logging.h"

#include "spotifymessages.pb.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

SpotifyServer::SpotifyServer(QObject* parent)
  : AbstractMessageHandler<pb::spotify::Message>(nullptr, parent),
    server_(new QTcpServer(this)),
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
  QTcpSocket* socket = server_->nextPendingConnection();
  SetDevice(socket);

  qLog(Info) << "Connection from port" << socket->peerPort();

  // Send any login messages that were queued before the client connected
  foreach (const pb::spotify::Message& message, queued_login_messages_) {
    SendOrQueueMessage(message);
  }
  queued_login_messages_.clear();

  // Don't take any more connections from clients
  disconnect(server_, SIGNAL(newConnection()), this, 0);
}

void SpotifyServer::SendOrQueueMessage(const pb::spotify::Message& message) {
  const bool is_login_message = message.has_login_request();

  QList<pb::spotify::Message>* queue =
      is_login_message ? &queued_login_messages_ : &queued_messages_;

  if (!device_ || (!is_login_message && !logged_in_)) {
    queue->append(message);
  } else {
    SendMessage(message);
  }
}

void SpotifyServer::Login(const QString& username, const QString& password,
                          pb::spotify::Bitrate bitrate, bool volume_normalisation) {
  pb::spotify::Message message;

  pb::spotify::LoginRequest* request = message.mutable_login_request();
  request->set_username(DataCommaSizeFromQString(username));
  if (!password.isEmpty()) {
    request->set_password(DataCommaSizeFromQString(password));
  }
  request->mutable_playback_settings()->set_bitrate(bitrate);
  request->mutable_playback_settings()->set_volume_normalisation(volume_normalisation);

  SendOrQueueMessage(message);
}

void SpotifyServer::SetPlaybackSettings(pb::spotify::Bitrate bitrate, bool volume_normalisation) {
  pb::spotify::Message message;

  pb::spotify::PlaybackSettings* request = message.mutable_set_playback_settings_request();
  request->set_bitrate(bitrate);
  request->set_volume_normalisation(volume_normalisation);

  SendOrQueueMessage(message);
}

void SpotifyServer::MessageArrived(const pb::spotify::Message& message) {
  if (message.has_login_response()) {
    const pb::spotify::LoginResponse& response = message.login_response();
    logged_in_ = response.success();

    if (response.success()) {
      // Send any messages that were queued before the client logged in
      foreach (const pb::spotify::Message& message, queued_messages_) {
        SendOrQueueMessage(message);
      }
      queued_messages_.clear();
    }

    emit LoginCompleted(response.success(), QStringFromStdString(response.error()),
                        response.error_code());
  } else if (message.has_playlists_updated()) {
    emit PlaylistsUpdated(message.playlists_updated());
  } else if (message.has_load_playlist_response()) {
    const pb::spotify::LoadPlaylistResponse& response = message.load_playlist_response();

    switch (response.request().type()) {
      case pb::spotify::Inbox:
        emit InboxLoaded(response);
        break;

      case pb::spotify::Starred:
        emit StarredLoaded(response);
        break;

      case pb::spotify::UserPlaylist:
        emit UserPlaylistLoaded(response);
        break;
    }
  } else if (message.has_playback_error()) {
    emit PlaybackError(QStringFromStdString(message.playback_error().error()));
  } else if (message.has_search_response()) {
    emit SearchResults(message.search_response());
  } else if (message.has_image_response()) {
    const pb::spotify::ImageResponse& response = message.image_response();
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
  } else if (message.has_browse_toplist_response()) {
    emit ToplistBrowseResults(message.browse_toplist_response());
  }
}

void SpotifyServer::LoadPlaylist(pb::spotify::PlaylistType type, int index) {
  pb::spotify::Message message;
  pb::spotify::LoadPlaylistRequest* req = message.mutable_load_playlist_request();

  req->set_type(type);
  if (index != -1) {
    req->set_user_playlist_index(index);
  }

  SendOrQueueMessage(message);
}

void SpotifyServer::SyncPlaylist(
    pb::spotify::PlaylistType type, int index, bool offline) {
  pb::spotify::Message message;
  pb::spotify::SyncPlaylistRequest* req = message.mutable_sync_playlist_request();
  req->mutable_request()->set_type(type);
  if (index != -1) {
    req->mutable_request()->set_user_playlist_index(index);
  }
  req->set_offline_sync(offline);

  SendOrQueueMessage(message);
}

void SpotifyServer::SyncInbox() {
  SyncPlaylist(pb::spotify::Inbox, -1, true);
}

void SpotifyServer::SyncStarred() {
  SyncPlaylist(pb::spotify::Starred, -1, true);
}

void SpotifyServer::SyncUserPlaylist(int index) {
  Q_ASSERT(index >= 0);
  SyncPlaylist(pb::spotify::UserPlaylist, index, true);
}

void SpotifyServer::LoadInbox() {
  LoadPlaylist(pb::spotify::Inbox);
}

void SpotifyServer::LoadStarred() {
  LoadPlaylist(pb::spotify::Starred);
}

void SpotifyServer::LoadUserPlaylist(int index) {
  Q_ASSERT(index >= 0);
  LoadPlaylist(pb::spotify::UserPlaylist, index);
}

void SpotifyServer::StartPlaybackLater(const QString& uri, quint16 port) {
  QTimer* timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));

  timer->start(100); // lol
  NewClosure(timer, SIGNAL(timeout()),
             this, SLOT(StartPlayback(QString,quint16)),
             uri, port);
}

void SpotifyServer::StartPlayback(const QString& uri, quint16 port) {
  pb::spotify::Message message;
  pb::spotify::PlaybackRequest* req = message.mutable_playback_request();

  req->set_track_uri(DataCommaSizeFromQString(uri));
  req->set_media_port(port);
  SendOrQueueMessage(message);
}

void SpotifyServer::Seek(qint64 offset_bytes) {
  pb::spotify::Message message;
  pb::spotify::SeekRequest* req = message.mutable_seek_request();

  req->set_offset_bytes(offset_bytes);
  SendOrQueueMessage(message);
}

void SpotifyServer::Search(const QString& text, int limit, int limit_album) {
  pb::spotify::Message message;
  pb::spotify::SearchRequest* req = message.mutable_search_request();

  req->set_query(DataCommaSizeFromQString(text));
  req->set_limit(limit);
  req->set_limit_album(limit_album);
  SendOrQueueMessage(message);
}

void SpotifyServer::LoadImage(const QString& id) {
  pb::spotify::Message message;
  pb::spotify::ImageRequest* req = message.mutable_image_request();

  req->set_id(DataCommaSizeFromQString(id));
  SendOrQueueMessage(message);
}

void SpotifyServer::AlbumBrowse(const QString& uri) {
  pb::spotify::Message message;
  pb::spotify::BrowseAlbumRequest* req = message.mutable_browse_album_request();

  req->set_uri(DataCommaSizeFromQString(uri));
  SendOrQueueMessage(message);
}

void SpotifyServer::LoadToplist() {
  pb::spotify::Message message;
  pb::spotify::BrowseToplistRequest* req = message.mutable_browse_toplist_request();
  req->set_type(pb::spotify::BrowseToplistRequest::Tracks);
  req->set_region(pb::spotify::BrowseToplistRequest::Everywhere);

  SendOrQueueMessage(message);
}
