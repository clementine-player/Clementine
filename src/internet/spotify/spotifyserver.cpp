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

#include "spotifyserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

#include "core/closure.h"
#include "core/logging.h"
#include "spotifymessages.pb.h"

SpotifyServer::SpotifyServer(QObject* parent)
    : AbstractMessageHandler<cpb::spotify::Message>(nullptr, parent),
      server_(new QTcpServer(this)),
      logged_in_(false) {
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
}

void SpotifyServer::Init() {
  if (!server_->listen(QHostAddress::LocalHost)) {
    qLog(Error) << "Couldn't open server socket" << server_->errorString();
  }
}

int SpotifyServer::server_port() const { return server_->serverPort(); }

void SpotifyServer::NewConnection() {
  QTcpSocket* socket = server_->nextPendingConnection();
  SetDevice(socket);

  qLog(Info) << "Connection from port" << socket->peerPort();

  // Send any login messages that were queued before the client connected
  for (const cpb::spotify::Message& message : queued_login_messages_) {
    SendOrQueueMessage(message);
  }
  queued_login_messages_.clear();

  // Don't take any more connections from clients
  disconnect(server_, SIGNAL(newConnection()), this, 0);
}

void SpotifyServer::SendOrQueueMessage(const cpb::spotify::Message& message) {
  const bool is_login_message = message.has_login_request();

  QList<cpb::spotify::Message>* queue =
      is_login_message ? &queued_login_messages_ : &queued_messages_;

  if (!device_ || (!is_login_message && !logged_in_)) {
    queue->append(message);
  } else {
    SendMessage(message);
  }
}

void SpotifyServer::Login(const QString& username, const QString& password,
                          cpb::spotify::Bitrate bitrate,
                          bool volume_normalisation) {
  cpb::spotify::Message message;

  cpb::spotify::LoginRequest* request = message.mutable_login_request();
  request->set_username(DataCommaSizeFromQString(username));
  if (!password.isEmpty()) {
    request->set_password(DataCommaSizeFromQString(password));
  }
  request->mutable_playback_settings()->set_bitrate(bitrate);
  request->mutable_playback_settings()->set_volume_normalisation(
      volume_normalisation);

  SendOrQueueMessage(message);
}

void SpotifyServer::SetPlaybackSettings(cpb::spotify::Bitrate bitrate,
                                        bool volume_normalisation) {
  cpb::spotify::Message message;

  cpb::spotify::PlaybackSettings* request =
      message.mutable_set_playback_settings_request();
  request->set_bitrate(bitrate);
  request->set_volume_normalisation(volume_normalisation);

  SendOrQueueMessage(message);
}

void SpotifyServer::MessageArrived(const cpb::spotify::Message& message) {
  if (message.has_login_response()) {
    const cpb::spotify::LoginResponse& response = message.login_response();
    logged_in_ = response.success();

    if (response.success()) {
      // Send any messages that were queued before the client logged in
      for (const cpb::spotify::Message& message : queued_messages_) {
        SendOrQueueMessage(message);
      }
      queued_messages_.clear();
    }

    emit LoginCompleted(response.success(),
                        QStringFromStdString(response.error()),
                        response.error_code());
  } else if (message.has_playlists_updated()) {
    emit PlaylistsUpdated(message.playlists_updated());
  } else if (message.has_load_playlist_response()) {
    const cpb::spotify::LoadPlaylistResponse& response =
        message.load_playlist_response();

    switch (response.request().type()) {
      case cpb::spotify::Inbox:
        emit InboxLoaded(response);
        break;

      case cpb::spotify::Starred:
        emit StarredLoaded(response);
        break;

      case cpb::spotify::UserPlaylist:
        emit UserPlaylistLoaded(response);
        break;
    }
  } else if (message.has_playback_error()) {
    emit PlaybackError(QStringFromStdString(message.playback_error().error()));
  } else if (message.has_search_response()) {
    emit SearchResults(message.search_response());
  } else if (message.has_image_response()) {
    const cpb::spotify::ImageResponse& response = message.image_response();
    const QString id = QStringFromStdString(response.id());

    if (response.has_data()) {
      emit ImageLoaded(
          id, QImage::fromData(
                  QByteArray(response.data().data(), response.data().size())));
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

void SpotifyServer::LoadPlaylist(cpb::spotify::PlaylistType type, int index) {
  cpb::spotify::Message message;
  cpb::spotify::LoadPlaylistRequest* req =
      message.mutable_load_playlist_request();

  req->set_type(type);
  if (index != -1) {
    req->set_user_playlist_index(index);
  }

  SendOrQueueMessage(message);
}

void SpotifyServer::SyncPlaylist(cpb::spotify::PlaylistType type, int index,
                                 bool offline) {
  cpb::spotify::Message message;
  cpb::spotify::SyncPlaylistRequest* req =
      message.mutable_sync_playlist_request();
  req->mutable_request()->set_type(type);
  if (index != -1) {
    req->mutable_request()->set_user_playlist_index(index);
  }
  req->set_offline_sync(offline);

  SendOrQueueMessage(message);
}

void SpotifyServer::SyncInbox() { SyncPlaylist(cpb::spotify::Inbox, -1, true); }

void SpotifyServer::SyncStarred() {
  SyncPlaylist(cpb::spotify::Starred, -1, true);
}

void SpotifyServer::SyncUserPlaylist(int index) {
  Q_ASSERT(index >= 0);
  SyncPlaylist(cpb::spotify::UserPlaylist, index, true);
}

void SpotifyServer::LoadInbox() { LoadPlaylist(cpb::spotify::Inbox); }

void SpotifyServer::LoadStarred() { LoadPlaylist(cpb::spotify::Starred); }

void SpotifyServer::LoadUserPlaylist(int index) {
  Q_ASSERT(index >= 0);
  LoadPlaylist(cpb::spotify::UserPlaylist, index);
}

void SpotifyServer::AddSongsToStarred(const QList<QUrl>& songs_urls) {
  AddSongsToPlaylist(cpb::spotify::Starred, songs_urls);
}

void SpotifyServer::AddSongsToUserPlaylist(int playlist_index,
                                           const QList<QUrl>& songs_urls) {
  AddSongsToPlaylist(cpb::spotify::UserPlaylist, songs_urls, playlist_index);
}

void SpotifyServer::AddSongsToPlaylist(
    const cpb::spotify::PlaylistType playlist_type,
    const QList<QUrl>& songs_urls, int playlist_index) {
  cpb::spotify::Message message;
  cpb::spotify::AddTracksToPlaylistRequest* req =
      message.mutable_add_tracks_to_playlist();
  req->set_playlist_type(playlist_type);
  req->set_playlist_index(playlist_index);
  for (const QUrl& song_url : songs_urls) {
    req->add_track_uri(DataCommaSizeFromQString(song_url.toString()));
  }
  SendOrQueueMessage(message);
}

void SpotifyServer::RemoveSongsFromStarred(
    const QList<int>& songs_indices_to_remove) {
  RemoveSongsFromPlaylist(cpb::spotify::Starred, songs_indices_to_remove);
}

void SpotifyServer::RemoveSongsFromUserPlaylist(
    int playlist_index, const QList<int>& songs_indices_to_remove) {
  RemoveSongsFromPlaylist(cpb::spotify::UserPlaylist, songs_indices_to_remove,
                          playlist_index);
}

void SpotifyServer::RemoveSongsFromPlaylist(
    const cpb::spotify::PlaylistType playlist_type,
    const QList<int>& songs_indices_to_remove, int playlist_index) {
  cpb::spotify::Message message;
  cpb::spotify::RemoveTracksFromPlaylistRequest* req =
      message.mutable_remove_tracks_from_playlist();
  req->set_playlist_type(playlist_type);
  if (playlist_type == cpb::spotify::UserPlaylist) {
    req->set_playlist_index(playlist_index);
  }
  for (int song_index : songs_indices_to_remove) {
    req->add_track_index(song_index);
  }
  SendOrQueueMessage(message);
}

void SpotifyServer::StartPlayback(const QString& uri, quint16 port) {
  cpb::spotify::Message message;
  cpb::spotify::PlaybackRequest* req = message.mutable_playback_request();

  req->set_track_uri(DataCommaSizeFromQString(uri));
  req->set_media_port(port);
  SendOrQueueMessage(message);
}

void SpotifyServer::Seek(qint64 offset_nsec) {
  cpb::spotify::Message message;
  cpb::spotify::SeekRequest* req = message.mutable_seek_request();

  req->set_offset_nsec(offset_nsec);
  SendOrQueueMessage(message);
}

void SpotifyServer::Search(const QString& text, int limit, int limit_album) {
  cpb::spotify::Message message;
  cpb::spotify::SearchRequest* req = message.mutable_search_request();

  req->set_query(DataCommaSizeFromQString(text));
  req->set_limit(limit);
  req->set_limit_album(limit_album);
  SendOrQueueMessage(message);
}

void SpotifyServer::LoadImage(const QString& id) {
  cpb::spotify::Message message;
  cpb::spotify::ImageRequest* req = message.mutable_image_request();

  req->set_id(DataCommaSizeFromQString(id));
  SendOrQueueMessage(message);
}

void SpotifyServer::AlbumBrowse(const QString& uri) {
  cpb::spotify::Message message;
  cpb::spotify::BrowseAlbumRequest* req =
      message.mutable_browse_album_request();

  req->set_uri(DataCommaSizeFromQString(uri));
  SendOrQueueMessage(message);
}

void SpotifyServer::LoadToplist() {
  cpb::spotify::Message message;
  cpb::spotify::BrowseToplistRequest* req =
      message.mutable_browse_toplist_request();
  req->set_type(cpb::spotify::BrowseToplistRequest::Tracks);
  req->set_region(cpb::spotify::BrowseToplistRequest::Everywhere);

  SendOrQueueMessage(message);
}

void SpotifyServer::SetPaused(const bool paused) {
  cpb::spotify::Message message;
  cpb::spotify::PauseRequest* req = message.mutable_pause_request();
  req->set_paused(paused);
  SendOrQueueMessage(message);
}
