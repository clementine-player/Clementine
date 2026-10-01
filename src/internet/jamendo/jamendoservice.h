/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2014, Chocobozzz <florian.bigard@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_JAMENDO_JAMENDOSERVICE_H_
#define INTERNET_JAMENDO_JAMENDOSERVICE_H_

#include <QJsonArray>
#include <QPersistentModelIndex>
#include <QUrlQuery>
#include <memory>

#include "core/song.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"

class JamendoUrlHandler;
class NetworkAccessManager;

class QLineEdit;
class QMenu;
class QNetworkReply;

// Jamendo's catalogue, browsed live through its API
// (https://developer.jamendo.com/v3.0). Its terms don't allow keeping a copy
// of the catalogue, and every Clementine user shares one client ID's request
// quota, so lists are only fetched when they're opened, and searches when
// they're entered.
class JamendoService : public InternetService {
  Q_OBJECT

 public:
  JamendoService(Application* app, InternetModel* parent);
  ~JamendoService() override;

  enum Type {
    // A list of tracks: its query is in Role_Query.
    Type_Tracks = InternetModel::TypeCount,
    // A list of albums: its query is in Role_Query.
    Type_Albums,
    // An album's tracks: its id is in Role_Id.
    Type_Album,
    // The genres.
    Type_Genres,
  };

  enum Role {
    Role_Query = InternetModel::RoleCount,
    Role_Id,
    // The item's page on jamendo.com.
    Role_ShareUrl,
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kUrlScheme;

  QStandardItem* CreateRootItem() override;
  void LazyPopulate(QStandardItem* item) override;
  void ShowContextMenu(const QPoint& global_pos) override;
  QWidget* HeaderWidget() const override;

  // The URL a track plays from: Jamendo's storage, from its id.
  static QUrl StreamUrl(const QString& track_id);

 private slots:
  void Search();
  void OpenShareUrl();
  void Refresh();
  void Homepage();

 private:
  void PopulateRoot();
  void Fetch(QStandardItem* item);
  void Request(QStandardItem* item, const QString& endpoint,
               const QUrlQuery& query, int retries_left);
  void RequestFinished(QNetworkReply* reply, const QPersistentModelIndex& index,
                       const QString& endpoint, const QUrlQuery& query,
                       int retries_left, int task_id);
  void AddTracks(QStandardItem* parent, const QJsonArray& tracks,
                 bool with_artist);
  void AddAlbums(QStandardItem* parent, const QJsonArray& albums);
  static QJsonArray AlbumTracks(const QJsonObject& album);
  Song TrackToSong(const QJsonObject& track) const;
  QStandardItem* CreateList(const QString& text, Type type,
                            const QString& query);

 private:
  NetworkAccessManager* network_;
  JamendoUrlHandler* url_handler_;

  QStandardItem* root_;
  QStandardItem* search_results_;
  QLineEdit* search_box_;

  std::unique_ptr<QMenu> context_menu_;
  QAction* open_share_url_;
  QAction* refresh_;
  QPersistentModelIndex context_index_;
};

#endif  // INTERNET_JAMENDO_JAMENDOSERVICE_H_
