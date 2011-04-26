#ifndef SPOTIFYSERVICE_H
#define SPOTIFYSERVICE_H

#include "radiomodel.h"
#include "radioservice.h"
#include "spotifyblob/spotifymessages.pb.h"

#include <QProcess>
#include <QTimer>

class SpotifyServer;

class SpotifyService : public RadioService {
  Q_OBJECT

public:
  SpotifyService(RadioModel* parent);
  ~SpotifyService();

  enum Type {
    Type_StarredPlaylist = RadioModel::TypeCount,
    Type_InboxPlaylist,
    Type_UserPlaylist,
    Type_Track,
  };

  enum Role {
    Role_UserPlaylistIndex = RadioModel::RoleCount,
    Role_Metadata,
  };

  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* parent);

  void Login(const QString& username, const QString& password);

  static const char* kServiceName;
  static const char* kSettingsGroup;

signals:
  void LoginFinished(bool success);

protected:
  virtual QModelIndex GetCurrentIndex();

private:
  void EnsureServerCreated(const QString& username = QString(),
                           const QString& password = QString());
  void FillPlaylist(QStandardItem* item, const protobuf::LoadPlaylistResponse& response);
  void SongFromProtobuf(const protobuf::Track& track, Song* song) const;

private slots:
  void BlobProcessError(QProcess::ProcessError error);
  void LoginCompleted(bool success);
  void PlaylistsUpdated(const protobuf::Playlists& response);
  void InboxLoaded(const protobuf::LoadPlaylistResponse& response);
  void StarredLoaded(const protobuf::LoadPlaylistResponse& response);
  void UserPlaylistLoaded(const protobuf::LoadPlaylistResponse& response);

private:
  SpotifyServer* server_;

  QString blob_path_;
  QProcess* blob_process_;

  QStandardItem* root_;
  QStandardItem* starred_;
  QStandardItem* inbox_;
  QList<QStandardItem*> playlists_;

  int login_task_id_;
};

#endif
