/* This file is part of Clementine.
   Copyright 2015, Marco Kirchner <kirchnermarco@gmail.com>
   Copyright 2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_PLAYMUSIC_PLAYMUSICSERVICE_H_
#define INTERNET_PLAYMUSIC_PLAYMUSICSERVICE_H_

#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"

class NetworkAccessManager;
class SearchBoxWidget;

class QMenu;
class QNetworkReply;

class PlayMusicService : public InternetService {
  Q_OBJECT

 public:
  PlayMusicService(Application* app, InternetModel* parent);
  ~PlayMusicService();

  // Internet Service methods
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);

  void ShowContextMenu(const QPoint& global_pos);
  QWidget* HeaderWidget() const;

  void Login(const QString &username, const QString &password);
  bool IsLoggedIn();
  void Logout();

  void LoadStreamUrl(QString id);

  int SimpleSearch(const QString& query);

  static const char* kServiceName;
  static const char* kSettingsGroup;

signals:
  void SimpleSearchResults(int id, SongList songs);
  void LoginStateChanged();
  void LoadStreamUrlComplete(const QUrl &url, const QString &id);

 public slots:
  void ShowConfig();

 private slots:
  void LoginFinished(QNetworkReply *reply);
  void LoadWebTokenFinished(QNetworkReply *reply);
  void LoadDeviceIdFinished(QNetworkReply *reply);
  void LoadStreamUrlFinished(QNetworkReply *reply, const QString &id);
  void Search(const QString& text, bool now = false);
  void DoSearch();
  void SearchFinished(QNetworkReply* reply, int task);
  void SimpleSearchFinished(QNetworkReply* reply, int id);
  void Homepage();

private:

  void LoadAccessTokenIfEmpty();
  void LoadWebToken();
  void LoadDeviceId();
  void ClearSearchResults();
  void EnsureItemsCreated();
  void EnsureMenuCreated();

  QNetworkReply* CreateRequest(const QString& ressource_name,
                               const QList<QPair<QString, QString>>& params);

  QVariant ExtractResult(QNetworkReply* reply);
  SongList ExtractSongs(const QVariant& result);
  Song ExtractSong(const QVariantMap& result_song);

  static QString GenerateSalt(int len);

  QStandardItem* root_;
  QStandardItem* search_;

  NetworkAccessManager* network_;

  QMenu* context_menu_;
  SearchBoxWidget* search_box_;
  QTimer* search_delay_;
  QString pending_search_;
  // Request IDs
  int next_pending_search_id_;

  QString access_token_;
  QString xt_;
  QString device_id_;
  QString key_;
  QDateTime expiry_time_;

  static const char* kBaseUrl;
  static const char* kWebUrl;
  static const char* kAuthUrl;
  static const char* kS1;
  static const char* kS2;

  static const int kSongSearchLimit;
  static const int kSearchDelayMsec;
};

#endif  // INTERNET_PLAYMUSIC_PLAYMUSICSERVICE_H_
