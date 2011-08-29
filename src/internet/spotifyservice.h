#ifndef SPOTIFYSERVICE_H
#define SPOTIFYSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"
#include "spotifyblob/common/spotifymessages.pb.h"

#include <QProcess>
#include <QTimer>

#include <boost/shared_ptr.hpp>

class Playlist;
class SpotifyServer;
class SpotifyUrlHandler;

class QMenu;

class SpotifyService : public InternetService {
  Q_OBJECT

public:
  SpotifyService(InternetModel* parent);
  ~SpotifyService();

  enum Type {
    Type_SearchResults = InternetModel::TypeCount,
    Type_StarredPlaylist,
    Type_InboxPlaylist,
    Type_UserPlaylist,
    Type_Track,
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
    LoginState_OtherError = 5
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kBlobDownloadUrl;
  static const int kSearchDelayMsec;

  void ReloadSettings();

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);
  void ItemDoubleClicked(QStandardItem* item);
  PlaylistItem::Options playlistitem_options() const;

  void Logout();
  void Login(const QString& username, const QString& password);
  void Search(const QString& text, Playlist* playlist, bool now = false);
  Q_INVOKABLE void LoadImage(const QUrl& url);

  SpotifyServer* server() const;

  bool IsBlobInstalled() const;
  void InstallBlob();

  // Persisted in the settings and updated on each Login().
  LoginState login_state() const { return login_state_; }

  static void SongFromProtobuf(const spotify_pb::Track& track, Song* song);

signals:
  void BlobStateChanged();
  void LoginFinished(bool success);
  void ImageLoaded(const QUrl& url, const QImage& image);

protected:
  virtual QModelIndex GetCurrentIndex();

private:
  void EnsureServerCreated(const QString& username = QString(),
                           const QString& password = QString());
  void StartBlobProcess();
  void FillPlaylist(QStandardItem* item, const spotify_pb::LoadPlaylistResponse& response);
  void EnsureMenuCreated();

  QStandardItem* PlaylistBySpotifyIndex(int index) const;
  bool DoPlaylistsDiffer(const spotify_pb::Playlists& response);

private slots:
  void BlobProcessError(QProcess::ProcessError error);
  void LoginCompleted(bool success, const QString& error,
                      spotify_pb::LoginResponse_Error error_code);
  void PlaylistsUpdated(const spotify_pb::Playlists& response);
  void InboxLoaded(const spotify_pb::LoadPlaylistResponse& response);
  void StarredLoaded(const spotify_pb::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const spotify_pb::LoadPlaylistResponse& response);
  void SearchResults(const spotify_pb::SearchResponse& response);
  void ImageLoaded(const QString& id, const QImage& image);
  void SyncPlaylistProgress(const spotify_pb::SyncPlaylistProgress& progress);

  void OpenSearchTab();
  void DoSearch();

  void ShowConfig();
  void SyncPlaylist();
  void BlobDownloadFinished();

private:
  SpotifyServer* server_;
  SpotifyUrlHandler* url_handler_;

  QString system_blob_path_;
  QString local_blob_version_;
  QString local_blob_path_;
  QProcess* blob_process_;

  QStandardItem* root_;
  QStandardItem* search_;
  QStandardItem* starred_;
  QStandardItem* inbox_;
  QList<QStandardItem*> playlists_;

  int login_task_id_;
  QString pending_search_;
  Playlist* pending_search_playlist_;

  QMenu* context_menu_;
  QMenu* playlist_context_menu_;
  QAction* playlist_sync_action_;
  QModelIndex context_item_;

  QTimer* search_delay_;

  int inbox_sync_id_;
  int starred_sync_id_;
  QMap<int, int> playlist_sync_ids_;

  LoginState login_state_;
};

#endif
