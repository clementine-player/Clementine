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

#ifndef INTERNET_ICECAST_ICECASTSERVICE_H_
#define INTERNET_ICECAST_ICECASTSERVICE_H_

#include <QXmlStreamReader>
#include <memory>

#include "icecastbackend.h"
#include "internet/core/internetservice.h"

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
  ~IcecastService() override;

  static const char* kServiceName;
  static const char* kDirectoryUrl;
  static const char* kHomepage;

  enum ItemType {
    Type_Stream = 3000,
    Type_Genre,
  };

  QStandardItem* CreateRootItem() override;
  void LazyPopulate(QStandardItem* item) override;

  void ShowContextMenu(const QPoint& global_pos) override;

  QWidget* HeaderWidget() const override;

 private slots:
  void LoadDirectory();
  void Homepage();
  void DownloadDirectoryFinished(QNetworkReply* reply, int task_id);
  void ParseDirectoryFinished(QFuture<IcecastBackend::StationList> future,
                              int task_id);

 private:
  void RequestDirectory(const QUrl& url, int task_id);
  void EnsureMenuCreated();
  IcecastBackend::StationList ParseDirectory(QIODevice* device) const;
  IcecastBackend::Station ReadStation(QXmlStreamReader* reader) const;

  QStandardItem* root_;
  NetworkAccessManager* network_;

  std::shared_ptr<IcecastBackend> backend_;
  IcecastModel* model_;
  IcecastFilterWidget* filter_;
};

#endif  // INTERNET_ICECAST_ICECASTSERVICE_H_
