/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2010, 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
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

#ifndef INTERNET_ICECASTSERVICE_H_
#define INTERNET_ICECASTSERVICE_H_

#include "internetservice.h"

#include <QXmlStreamReader>

#include "icecastbackend.h"

class IcecastFilterWidget;
class IcecastModel;
class NetworkAccessManager;

class QAction;
class QMenu;
class QNetworkReply;

class IcecastService : public InternetService {
  Q_OBJECT

 public:
  IcecastService(Application* app, InternetModel* parent);
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

  void ShowContextMenu(const QPoint& global_pos);

  QWidget* HeaderWidget() const;

 private slots:
  void LoadDirectory();
  void Homepage();
  void DownloadDirectoryFinished(QNetworkReply* reply);
  void ParseDirectoryFinished(QFuture<IcecastBackend::StationList> future);

 private:
  void RequestDirectory(const QUrl& url);
  void EnsureMenuCreated();
  IcecastBackend::StationList ParseDirectory(QIODevice* device) const;
  IcecastBackend::Station ReadStation(QXmlStreamReader* reader) const;

  QStandardItem* root_;
  NetworkAccessManager* network_;
  QMenu* context_menu_;

  IcecastBackend* backend_;
  IcecastModel* model_;
  IcecastFilterWidget* filter_;

  int load_directory_task_id_;
};

#endif  // INTERNET_ICECASTSERVICE_H_
