#ifndef SPOTIFYSERVICE_H
#define SPOTIFYSERVICE_H

#include "radiomodel.h"
#include "radioservice.h"
#include "spotifyblob/spotifymessages.pb.h"

#include <QProcess>
#include <QTimer>

#include <boost/shared_ptr.hpp>

class LibraryBackend;
class LibraryModel;
class SpotifyServer;
class SpotifyUrlHandler;

class QMenu;
class QSortFilterProxyModel;
class QTemporaryFile;

class SpotifyService : public RadioService {
  Q_OBJECT

public:
  SpotifyService(RadioModel* parent);
  ~SpotifyService();

  enum Type {
    Type_SearchResults = RadioModel::TypeCount,
    Type_StarredPlaylist,
    Type_InboxPlaylist,
    Type_UserPlaylist,
    Type_Track,
  };

  enum Role {
    Role_UserPlaylistIndex = RadioModel::RoleCount,
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kSearchSongsTable;
  static const char* kSearchFtsTable;

  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* parent);

  void Login(const QString& username, const QString& password);

  PlaylistItem::Options playlistitem_options() const;

  QWidget* HeaderWidget() const;

  SpotifyServer* server() const;

signals:
  void LoginFinished(bool success);

protected:
  virtual QModelIndex GetCurrentIndex();

private:
  void EnsureServerCreated(const QString& username = QString(),
                           const QString& password = QString());
  void FillPlaylist(QStandardItem* item, const protobuf::LoadPlaylistResponse& response);
  void SongFromProtobuf(const protobuf::Track& track, Song* song) const;
  void EnsureMenuCreated();

  QStandardItem* PlaylistBySpotifyIndex(int index) const;
  bool DoPlaylistsDiffer(const protobuf::Playlists& response);

private slots:
  void BlobProcessError(QProcess::ProcessError error);
  void LoginCompleted(bool success);
  void PlaylistsUpdated(const protobuf::Playlists& response);
  void InboxLoaded(const protobuf::LoadPlaylistResponse& response);
  void StarredLoaded(const protobuf::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const protobuf::LoadPlaylistResponse& response);
  void SearchResults(const protobuf::SearchResponse& response);

  void Search(const QString& text);

private:
  SpotifyServer* server_;
  SpotifyUrlHandler* url_handler_;

  QString blob_path_;
  QProcess* blob_process_;

  QStandardItem* root_;
  QStandardItem* search_results_;
  QStandardItem* starred_;
  QStandardItem* inbox_;
  QList<QStandardItem*> playlists_;

  int login_task_id_;
  QString pending_search_;

  QMenu* context_menu_;
  QModelIndex context_item_;

  QTemporaryFile* database_file_;
  boost::shared_ptr<Database> database_;
  LibraryBackend* library_backend_;
  LibraryFilterWidget* library_filter_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;
};

#endif
