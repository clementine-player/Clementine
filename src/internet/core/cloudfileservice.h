/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef INTERNET_CORE_CLOUDFILESERVICE_H_
#define INTERNET_CORE_CLOUDFILESERVICE_H_

#include <QMenu>
#include <memory>

#include "core/tagreaderclient.h"
#include "internet/core/internetservice.h"
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
  CloudFileService(Application* app, InternetModel* parent,
                   const QString& service_name, const QString& service_id,
                   const QIcon& icon, SettingsDialog::Page settings_page);

  // InternetService
  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* item);

  virtual bool has_credentials() const = 0;
  bool is_indexing() const { return indexing_task_id_ != -1; }

 signals:
  void AllIndexingTasksFinished();

 public slots:
  void ShowSettingsDialog();

 protected:
  virtual void Connect() = 0;
  virtual bool ShouldIndexFile(const QUrl& url, const QString& mime_type) const;
  virtual void MaybeAddFileToDatabase(const Song& metadata,
                                      const QString& mime_type,
                                      const QUrl& download_url,
                                      const QString& authorisation);
  virtual bool IsSupportedMimeType(const QString& mime_type) const;
  QString GuessMimeTypeForFile(const QString& filename) const;
  void AbortReadTagsReplies();

  // Called once when context menu is created
  virtual void PopulateContextMenu() override;

 protected slots:
  void ShowCoverManager();
  void AddToPlaylist(QMimeData* mime);
  void ReadTagsFinished(TagReaderClient::ReplyType* reply,
                        const Song& metadata);
  void FullRescanRequested();
  virtual void DoFullRescan() {}

 protected:
  QStandardItem* root_;
  NetworkAccessManager* network_;

  std::shared_ptr<LibraryBackend> library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;

  std::unique_ptr<AlbumCoverManager> cover_manager_;
  PlaylistManager* playlist_manager_;
  TaskManager* task_manager_;
  QList<TagReaderClient::ReplyType*> pending_tagreader_replies_;

 private:
  QIcon icon_;
  SettingsDialog::Page settings_page_;

  int indexing_task_id_;
  int indexing_task_progress_;
  int indexing_task_max_;
};

#endif  // INTERNET_CORE_CLOUDFILESERVICE_H_
