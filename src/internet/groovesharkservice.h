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

#ifndef GROOVESHARKSERVICE_H
#define GROOVESHARKSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"

class NetworkAccessManager;
class Playlist;
class QMenu;
class QSortFilterProxyModel;
class QNetworkRequest;

class GrooveSharkService : public InternetService {
  Q_OBJECT
 public:
  GrooveSharkService(InternetModel *parent);
  ~GrooveSharkService();

  enum Type {
    Type_SearchResults = InternetModel::TypeCount
  };
  // Internet Service methods
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem *parent);

  void ItemDoubleClicked(QStandardItem* item);
  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  void Search(const QString& text, Playlist* playlist, bool now = false);

  static const char* kServiceName;
  static const char* kUrl;

  static const int kSongSearchLimit;

  static const char* kApiKey;
  static const char* kApiSecret;

 protected:
  QModelIndex GetCurrentIndex();

 private slots:
  void UpdateTotalSongCount(int count);

  void SearchSongsFinished();

 private:
  void EnsureMenuCreated();
  void EnsureConnected();

  void OpenSearchTab();

  // Create a request for the given method, with the given params.
  // If need_authentication is true, add session_id to params.
  // Returns the reply object created
  QNetworkReply *CreateRequest(const QString& method_name, const QList<QPair<QString, QString> > params,
                     bool need_authentication = false);

  Playlist* pending_search_playlist_;

  QStandardItem* root_;
  QStandardItem* search_;

  NetworkAccessManager* network_;

  QMenu* context_menu_;
  QModelIndex context_item_;

  QString session_id_;
  QByteArray api_key_;

};


#endif // GROOVESHARKSERVICE_H
