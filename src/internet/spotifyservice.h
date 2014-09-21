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
    Type_Track,
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

  void ReloadSettings();

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  void ShowContextMenu(const QPoint& global_pos);
  void ItemDoubleClicked(QStandardItem* item);
  void DropMimeData(const QMimeData* data, const QModelIndex& index);
  QWidget* HeaderWidget() const;

  void Logout();
  void Login(const QString& username, const QString& password);
  Q_INVOKABLE void LoadImage(const QString& id);
  Q_INVOKABLE void SetPaused(const bool paused);
  Q_INVOKABLE void Seek(const int offset /* in msec */);

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

 private:
  void StartBlobProcess();
  void FillPlaylist(
      QStandardItem* item,
      const google::protobuf::RepeatedPtrField<pb::spotify::Track>& tracks);
  void FillPlaylist(QStandardItem* item,
                    const pb::spotify::LoadPlaylistResponse& response);
  void EnsureMenuCreated();
  void ClearSearchResults();

  QStandardItem* PlaylistBySpotifyIndex(int index) const;
  bool DoPlaylistsDiffer(const pb::spotify::Playlists& response) const;

 private slots:
  void EnsureServerCreated(const QString& username = QString(),
                           const QString& password = QString());
  void BlobProcessError(QProcess::ProcessError error);
  void LoginCompleted(bool success, const QString& error,
                      pb::spotify::LoginResponse_Error error_code);
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
  QAction* playlist_sync_action_;

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
