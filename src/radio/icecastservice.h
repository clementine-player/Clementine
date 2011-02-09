/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef ICECASTSERVICE_H
#define ICECASTSERVICE_H

#include "icecastbackend.h"
#include "radioservice.h"

#include <QXmlStreamReader>

class IcecastFilterWidget;
class IcecastModel;
class NetworkAccessManager;

class QAction;
class QMenu;

class IcecastService : public RadioService {
  Q_OBJECT
 public:
  IcecastService(RadioModel* parent);
  ~IcecastService();

  static const char* kServiceName;
  static const char* kDirectoryUrl;
  static const char* kHomepage;

  enum ItemType {
    Type_Stream = 3000,
    Type_Genre,
  };

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  QWidget* HeaderWidget() const;

 private slots:
  void LoadDirectory();
  void Homepage();
  void AddToPlaylist();
  void LoadToPlaylist();
  void OpenInNewPlaylist();

 private:
  void EnsureMenuCreated();
  IcecastBackend::StationList ParseDirectory(QIODevice* device) const;
  IcecastBackend::Station ReadStation(QXmlStreamReader* reader) const;

  QStandardItem* root_;
  NetworkAccessManager* network_;
  QMenu* context_menu_;
  QModelIndex context_item_;
  QAction* add_to_playlist_;
  QAction* load_to_playlist_;
  QAction* open_in_new_playlist_;

  IcecastBackend* backend_;
  IcecastModel* model_;
  IcecastFilterWidget* filter_;

  int load_directory_task_id_;

 private slots:
  void DownloadDirectoryFinished();
  void ParseDirectoryFinished();
};

#endif
