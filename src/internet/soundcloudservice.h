/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef SOUNDCLOUDSERVICE_H
#define SOUNDCLOUDSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"

class NetworkAccessManager;
class SearchBoxWidget;

class QMenu;
class QNetworkReply;

class SoundCloudService : public InternetService {
  Q_OBJECT
 public:
  SoundCloudService(Application* app, InternetModel* parent);
  ~SoundCloudService();

  // Internet Service methods
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);

  // TODO
  // QList<QAction*> playlistitem_actions(const Song& song);
  void ShowContextMenu(const QPoint& global_pos);
  QWidget* HeaderWidget() const;

  int SimpleSearch(const QString& query);

  static const char* kServiceName;
  static const char* kSettingsGroup;

signals:
  void SimpleSearchResults(int id, SongList songs);

 private slots:
  void Search(const QString& text, bool now = false);
  void DoSearch();
  void SearchFinished(QNetworkReply* reply, int task);
  void SimpleSearchFinished(QNetworkReply* reply, int id);

  void Homepage();

 private:
  void ClearSearchResults();
  void EnsureItemsCreated();
  void EnsureMenuCreated();
  QNetworkReply* CreateRequest(const QString& ressource_name,
                               const QList<QPair<QString, QString> >& params);
  // Convenient function for extracting result from reply
  QVariant ExtractResult(QNetworkReply* reply);
  SongList ExtractSongs(const QVariant& result);
  Song ExtractSong(const QVariantMap& result_song);

  QStandardItem* root_;
  QStandardItem* search_;

  NetworkAccessManager* network_;

  QMenu* context_menu_;
  SearchBoxWidget* search_box_;
  QTimer* search_delay_;
  QString pending_search_;
  int next_pending_search_id_;

  QByteArray api_key_;

  static const char* kUrl;
  static const char* kUrlCover;
  static const char* kHomepage;

  static const int kSongSearchLimit;
  static const int kSongSimpleSearchLimit;
  static const int kSearchDelayMsec;

  static const char* kApiClientId;
};

#endif  // SOUNDCLOUDSERVICE_H
