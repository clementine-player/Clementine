#ifndef SPOTIFYSERVICE_H
#define SPOTIFYSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"
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

  enum Role { Role_UserPlaylistIndex = InternetModel::RoleCount, };

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
  void ShowConfig();
  void RemoveCurrentFromPlaylist();

 private:
  void StartBlobProcess();
  void FillPlaylist(
      QStandardItem* item,
      const google::protobuf::RepeatedPtrField<pb::spotify::Track>& tracks);
  void FillPlaylist(QStandardItem* item,
                    const pb::spotify::LoadPlaylistResponse& response);
  void AddSongsToPlaylist(int playlist_index, const QList<QUrl>& songs_urls);
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
  void AddCurrentSongToPlaylist(QAction* action);
  void RemoveSongsFromPlaylist(int playlist_index,
                               const QList<int>& songs_indices_to_remove);
  void PlaylistsUpdated(const pb::spotify::Playlists& response);
  void InboxLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void StarredLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void SearchResults(const pb::spotify::SearchResponse& response);
  void SyncPlaylistProgress(const pb::spotify::SyncPlaylistProgress& progress);
  void ToplistLoaded(const pb::spotify::BrowseToplistResponse& response);

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
  QList<QAction*> playlistitem_actions_;
  QAction* remove_from_playlist_;
  QUrl current_song_url_;

  SearchBoxWidget* search_box_;

  QTimer* search_delay_;

  int inbox_sync_id_;
  int starred_sync_id_;
  QMap<int, int> playlist_sync_ids_;

  LoginState login_state_;
  pb::spotify::Bitrate bitrate_;
  bool volume_normalisation_;
};

#endif
