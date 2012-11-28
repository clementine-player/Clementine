#ifndef CLOUDFILESERVICE_H
#define CLOUDFILESERVICE_H

#include "internetservice.h"

#include <QMenu>

#include "ui/albumcovermanager.h"

class UrlHandler;
class QSortFilterProxyModel;
class LibraryBackend;
class LibraryModel;
class NetworkAccessManager;
class PlaylistManager;

class CloudFileService : public InternetService {
  Q_OBJECT
 public:
  CloudFileService(
      Application* app,
      InternetModel* parent,
      const QString& service_name,
      const QString& service_id,
      const QIcon& icon,
      SettingsDialog::Page settings_page);

  // InternetService
  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* item);
  virtual void ShowContextMenu(const QPoint& point);

 protected:
  virtual bool has_credentials() const = 0;
  virtual void Connect() = 0;

 private slots:
  void ShowCoverManager();
  void AddToPlaylist(QMimeData* mime);
  void ShowSettingsDialog();

 protected:
  QStandardItem* root_;
  NetworkAccessManager* network_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;

  boost::scoped_ptr<QMenu> context_menu_;
  boost::scoped_ptr<AlbumCoverManager> cover_manager_;
  PlaylistManager* playlist_manager_;

 private:
  QIcon icon_;
  SettingsDialog::Page settings_page_;
};

#endif  // CLOUDFILESERVICE_H
