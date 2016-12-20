/* This file is part of Clementine.
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011-2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011-2012, 2014, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_SPOTIFY_SPOTIFYSERVICE_H_
#define INTERNET_SPOTIFY_SPOTIFYSERVICE_H_

#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"
#include "spotifymessages.pb.h"

#include <QProcess>
#include <QTimer>

class Playlist;
class SearchBoxWidget;
class SpotifyServer;

class QMenu;

class SpotifyService : public InternetService {
  Q_OBJECT

 public:
  SpotifyService(Application* app, InternetModel* parent);
  ~SpotifyService();

  enum Type {
    Type_SearchResults = InternetModel::TypeCount,
    Type_StarredPlaylist,
    Type_InboxPlaylist,
    Type_Toplist,
  };

  enum Role {
    Role_UserPlaylistIndex = InternetModel::RoleCount,
  };

  // Values are persisted - don't change.
  enum LoginState {
    LoginState_LoggedIn = 1,
    LoginState_Banned = 2,
    LoginState_BadCredentials = 3,
    LoginState_NoPremium = 4,
    LoginState_OtherError = 5,
    LoginState_ReloginFailed = 6
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kBlobDownloadUrl;
  static const int kSearchDelayMsec;

  void ReloadSettings() override;

  QStandardItem* CreateRootItem() override;
  void LazyPopulate(QStandardItem* parent) override;
  void ShowContextMenu(const QPoint& global_pos) override;
  void ItemDoubleClicked(QStandardItem* item) override;
  void DropMimeData(const QMimeData* data, const QModelIndex& index) override;
  QList<QAction*> playlistitem_actions(const Song& song) override;
  PlaylistItem::Options playlistitem_options() const override;
  QWidget* HeaderWidget() const override;

  void Logout();
  void Login(const QString& username, const QString& password);
  Q_INVOKABLE void LoadImage(const QString& id);
  Q_INVOKABLE void SetPaused(bool paused);

  SpotifyServer* server() const;

  bool IsBlobInstalled() const;
  void InstallBlob();

  // Persisted in the settings and updated on each Login().
  LoginState login_state() const { return login_state_; }
  bool IsLoggedIn() const { return login_state_ == LoginState_LoggedIn; }

  static void SongFromProtobuf(const pb::spotify::Track& track, Song* song);

 signals:
  void BlobStateChanged();
  void LoginFinished(bool success);
  void ImageLoaded(const QString& id, const QImage& image);

 public slots:
  void Search(const QString& text, bool now = false);
  void ShowConfig() override;
  void RemoveCurrentFromPlaylist();

 private:
  void StartBlobProcess();
  void FillPlaylist(
      QStandardItem* item,
      const google::protobuf::RepeatedPtrField<pb::spotify::Track>& tracks);
  void FillPlaylist(QStandardItem* item,
                    const pb::spotify::LoadPlaylistResponse& response);
  void AddSongsToUserPlaylist(int playlist_index,
                              const QList<QUrl>& songs_urls);
  void AddSongsToStarred(const QList<QUrl>& songs_urls);
  void EnsureMenuCreated();
  // Create a new "show config" action. The caller is responsible for deleting
  // the pointer (or adding it to menu or anything else that will take ownership
  // of it)
  QAction* GetNewShowConfigAction();
  void ClearSearchResults();

  QStandardItem* PlaylistBySpotifyIndex(int index) const;
  bool DoPlaylistsDiffer(const pb::spotify::Playlists& response) const;

 private slots:
  void EnsureServerCreated(const QString& username = QString(),
                           const QString& password = QString());
  void BlobProcessError(QProcess::ProcessError error);
  void LoginCompleted(bool success, const QString& error,
                      pb::spotify::LoginResponse_Error error_code);
  void AddCurrentSongToUserPlaylist(QAction* action);
  void AddCurrentSongToStarredPlaylist();
  void RemoveSongsFromUserPlaylist(int playlist_index,
                                   const QList<int>& songs_indices_to_remove);
  void RemoveSongsFromStarred(const QList<int>& songs_indices_to_remove);
  void PlaylistsUpdated(const pb::spotify::Playlists& response);
  void InboxLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void StarredLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void SearchResults(const pb::spotify::SearchResponse& response);
  void SyncPlaylistProgress(const pb::spotify::SyncPlaylistProgress& progress);
  void ToplistLoaded(const pb::spotify::BrowseToplistResponse& response);
  void GetCurrentSongUrlToShare() const;
  void GetCurrentPlaylistUrlToShare() const;

  void DoSearch();

  void SyncPlaylist();
  void BlobDownloadFinished();

 private:
  SpotifyServer* server_;

  QString system_blob_path_;
  QString local_blob_version_;
  QString local_blob_path_;
  QProcess* blob_process_;

  QStandardItem* root_;
  QStandardItem* search_;
  QStandardItem* starred_;
  QStandardItem* inbox_;
  QStandardItem* toplist_;
  QList<QStandardItem*> playlists_;

  int login_task_id_;
  QString pending_search_;

  QMenu* context_menu_;
  QMenu* playlist_context_menu_;
  QMenu* song_context_menu_;
  QAction* playlist_sync_action_;
  QAction* get_url_to_share_playlist_;
  QList<QAction*> playlistitem_actions_;
  QAction* remove_from_playlist_;
  QUrl current_song_url_;
  QUrl current_playlist_url_;

  SearchBoxWidget* search_box_;

  QTimer* search_delay_;

  int inbox_sync_id_;
  int starred_sync_id_;
  QMap<int, int> playlist_sync_ids_;

  LoginState login_state_;
  pb::spotify::Bitrate bitrate_;
  bool volume_normalisation_;
};

#endif  // INTERNET_SPOTIFY_SPOTIFYSERVICE_H_
