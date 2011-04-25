#ifndef SPOTIFYSERVICE_H
#define SPOTIFYSERVICE_H

#include "radioservice.h"

#include <QProcess>
#include <QTimer>

class SpotifyServer;

class SpotifyService : public RadioService {
  Q_OBJECT

public:
  explicit SpotifyService(RadioModel* parent);
  virtual ~SpotifyService();

  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* parent);

  void Login(const QString& username, const QString& password);
  void Search(const QString& query);

  static const char* kServiceName;
  static const char* kSettingsGroup;

signals:
  void LoginFinished(bool success);

protected:
  virtual QModelIndex GetCurrentIndex();

private slots:
  void ClientConnected();
  void LoginCompleted(bool success);
  void BlobProcessError(QProcess::ProcessError error);

private:
  SpotifyServer* server_;

  QString blob_path_;
  QProcess* blob_process_;

  QString pending_username_;
  QString pending_password_;
};

#endif
