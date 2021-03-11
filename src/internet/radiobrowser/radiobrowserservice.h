/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#ifndef INTERNET_RADIOBROWSER_RADIOBROWSERSERVICE_H_
#define INTERNET_RADIOBROWSER_RADIOBROWSERSERVICE_H_

#include <QJsonObject>

#include "core/cachedlist.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"

class RadioBrowserUrlHandler;

class QNetworkAccessManager;
class QNetworkReply;
class QMenu;

class RadioBrowserService : public InternetService {
  Q_OBJECT

 public:
  RadioBrowserService(Application* app, InternetModel* parent);
  ~RadioBrowserService();

  enum ItemType {
    Type_Stream = 2000,
  };

  enum Type {
    Type_Category = InternetModel::TypeCount,
    Type_CategoryItem,
    Type_Top100,
  };

  enum Role {
    Role_ListUrl = InternetModel::RoleCount,
    Role_ItemsUrl,
    Role_StationUuid,
  };

  struct Stream {
    QString name_;
    QUrl url_;
    QString uuid_;
    QUrl favicon_;

    Song ToSong(const QString& prefix) const;
  };

  struct Branch {
    QString name;
    QString listUrl;
    QString itemsUrl;
    Type type;
  };

  static QList<Branch> BranchList;
  static QString SearchUrl;
  static QString PlayClickUrl;

  typedef QList<Stream> StreamList;

  static const char* kServiceName;
  static const char* kSettingsGroup;

  const QIcon& icon() const { return icon_; }

  QStandardItem* CreateRootItem();
  void ShowContextMenu(const QPoint& global_pos);

  PlaylistItem::Options playlistitem_options() const;

  void Search(int search_id, const QString& query, const int limit);
  void ItemNowPlaying(QStandardItem* item) override;

 signals:
  void SearchFinished(int search_id, RadioBrowserService::StreamList streams);

 public slots:
  void ReloadSettings();

 private slots:
  void LazyPopulate(QStandardItem* item);

  void RefreshRootItem();
  void RefreshCategory(QStandardItem* item);
  void RefreshCategoryItem(QStandardItem* item);
  void RefreshTop100(QStandardItem* item);

  void RefreshCategoryFinished(QNetworkReply* reply, int task_id,
                               QStandardItem* item);
  void RefreshStreamsFinished(QNetworkReply* reply, int task_id,
                              QStandardItem* item);
  void SearchFinishedInternal(QNetworkReply* reply, int task_id, int search_id);

  void Homepage();
  void ShowConfig();

 private:
  void ReadStation(QJsonObject& value, StreamList* ret);
  void PopulateCategory(QStandardItem* parentItem, QStringList& elements);
  void PopulateStreams(QStandardItem* parentItem, StreamList& streams);
  bool EnsureServerConfig();

 private:
  QStandardItem* root_;
  QMenu* context_menu_;

  QNetworkAccessManager* network_;

  const QString name_;
  QString main_server_url_;
  const QUrl homepage_url_;
  const QIcon icon_;
};

QDataStream& operator<<(QDataStream& out,
                        const RadioBrowserService::Stream& stream);
QDataStream& operator>>(QDataStream& in, RadioBrowserService::Stream& stream);
Q_DECLARE_METATYPE(RadioBrowserService::Stream)

#endif  // INTERNET_RADIOBROWSER_RADIOBROWSERSERVICE_H_
