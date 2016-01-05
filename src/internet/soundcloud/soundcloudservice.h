/* This file is part of Clementine.
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

#ifndef INTERNET_SOUNDCLOUD_SOUNDCLOUDSERVICE_H_
#define INTERNET_SOUNDCLOUD_SOUNDCLOUDSERVICE_H_

#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"

class NetworkAccessManager;
class OAuthenticator;
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

  // TODO(Arnaud Bienner)
  // QList<QAction*> playlistitem_actions(const Song& song);
  void ShowContextMenu(const QPoint& global_pos);
  QWidget* HeaderWidget() const;

  void Connect();
  bool IsLoggedIn();
  void Logout();

  int SimpleSearch(const QString& query);

  static const char* kServiceName;
  static const char* kSettingsGroup;

 signals:
  void SimpleSearchResults(int id, SongList songs);
  void Connected();

 public slots:
  void ShowConfig();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);
  void UserTracksRetrieved(QNetworkReply* reply);
  void UserActivitiesRetrieved(QNetworkReply* reply);
  void UserPlaylistsRetrieved(QNetworkReply* reply);
  void PlaylistRetrieved(QNetworkReply* reply, int request_id);
  void GetSelectedSongUrl() const;
  void Search(const QString& text, bool now = false);
  void DoSearch();
  void SearchFinished(QNetworkReply* reply, int task);
  void SimpleSearchFinished(QNetworkReply* reply, int id);

  void Homepage();

 private:
  struct PlaylistInfo {
    PlaylistInfo() {}
    PlaylistInfo(int id, QStandardItem* item) : id_(id), item_(item) {}

    int id_;
    QStandardItem* item_;
  };

  // Try to load "access_token" from preferences if the current access_token's
  // value is empty
  void LoadAccessTokenIfEmpty();
  void RetrieveUserData();
  void RetrieveUserTracks();
  void RetrieveUserActivities();
  void RetrieveUserPlaylists();
  void RetrievePlaylist(int playlist_id, QStandardItem* playlist_item);
  void ClearSearchResults();
  void EnsureItemsCreated();
  void EnsureMenuCreated();

  QStandardItem* CreatePlaylistItem(const QString& playlist_name);

  QNetworkReply* CreateRequest(const QString& ressource_name,
                               const QList<QPair<QString, QString>>& params);
  // Convenient function for extracting result from reply
  QVariant ExtractResult(QNetworkReply* reply);
  // Returns items directly, as activities can be playlists or songs
  QList<QStandardItem*> ExtractActivities(const QVariant& result);
  SongList ExtractSongs(const QVariant& result);
  Song ExtractSong(const QVariantMap& result_song);

  QStandardItem* root_;
  QStandardItem* search_;
  QStandardItem* user_tracks_;
  QStandardItem* user_playlists_;
  QStandardItem* user_activities_;

  NetworkAccessManager* network_;

  QMenu* context_menu_;
  SearchBoxWidget* search_box_;
  QTimer* search_delay_;
  QString pending_search_;
  // Request IDs
  int next_pending_search_id_;
  int next_retrieve_playlist_id_;

  QMap<int, PlaylistInfo> pending_playlists_requests_;

  QByteArray api_key_;

  QString access_token_;
  QDateTime expiry_time_;

  static const char* kUrl;
  static const char* kOAuthEndpoint;
  static const char* kOAuthTokenEndpoint;
  static const char* kOAuthScope;
  static const char* kHomepage;

  static const int kSongSearchLimit;
  static const int kSongSimpleSearchLimit;
  static const int kSearchDelayMsec;

  static const char* kApiClientId;
  static const char* kApiClientSecret;
};

#endif  // INTERNET_SOUNDCLOUD_SOUNDCLOUDSERVICE_H_
