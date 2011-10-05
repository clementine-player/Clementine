/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

class GroovesharkUrlHandler;
class NetworkAccessManager;
class Playlist;
class QMenu;
class QSortFilterProxyModel;
class QNetworkRequest;

class GroovesharkService : public InternetService {
  Q_OBJECT
 public:
  GroovesharkService(InternetModel *parent);
  ~GroovesharkService();

  enum Type {
    Type_SearchResults = InternetModel::TypeCount,
    Type_UserPlaylist,
    Type_Track
  };

  // Values are persisted - don't change.
  enum LoginState {
    LoginState_LoggedIn = 1,
    LoginState_AuthFailed = 2,
    LoginState_NoPremium = 3,
    LoginState_OtherError = 4
  };

  // Internet Service methods
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem *parent);

  void ItemDoubleClicked(QStandardItem* item);
  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  void Search(const QString& text, Playlist* playlist, bool now = false);
  // User should be logged in to be able to generate streaming urls
  QUrl GetStreamingUrlFromSongId(const QString& song_id,
                                 QString* server_id, QString* stream_key,
                                 qint64* length_nanosec);
  void Login(const QString& username, const QString& password);
  void Logout();
  bool IsLoggedIn() { return !session_id_.isEmpty(); }
  void RetrieveUserPlaylists();
  void MarkStreamKeyOver30Secs(const QString& stream_key, const QString& server_id);
  void MarkSongComplete(const QString& song_id, const QString& stream_key, const QString& server_id);

  // Persisted in the settings and updated on each Login().
  LoginState login_state() const { return login_state_; }
  const QString& session_id() { return session_id_; }

  int SimpleSearch(const QString& query);
  int SearchAlbums(const QString& query);
  void FetchSongsForAlbum(int id, const QUrl& url);
  void FetchSongsForAlbum(int id, quint64 album_id);

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kUrl;
  static const char* kUrlCover;

  static const int kSongSearchLimit;
  static const int kSearchDelayMsec;

  static const char* kApiKey;
  static const char* kApiSecret;

 signals:
  void LoginFinished(bool success);
  void SimpleSearchResults(int id, SongList songs);
  void AlbumSearchResult(int id, SongList songs);
  void AlbumSongsLoaded(int id, SongList songs);

 protected:
  QModelIndex GetCurrentIndex();

  struct PlaylistInfo {
    PlaylistInfo() {}
    PlaylistInfo(int id, QString name)
      : id_(id), name_(name) {}

    int id_;
    QString name_;
  };

 private slots:
  void UpdateTotalSongCount(int count);

  void ShowConfig();
  void SessionCreated();
  void OpenSearchTab();
  void DoSearch();
  void SearchSongsFinished();
  void SimpleSearchFinished();
  void SearchAlbumsFinished(QNetworkReply* reply, int id);
  void GetAlbumSongsFinished(QNetworkReply* reply, int id);
  void Authenticated();
  void UserPlaylistsRetrieved();
  void PlaylistSongsRetrieved();
  void StreamMarked();
  void SongMarkedAsComplete();


 private:
  void EnsureMenuCreated();
  void EnsureItemsCreated();
  void EnsureConnected();

  void AuthenticateSession();
  void InitCountry();

  // Create a request for the given method, with the given params.
  // If need_authentication is true, add session_id to params.
  // Returns the reply object created
  QNetworkReply* CreateRequest(const QString& method_name, const QList<QPair<QString, QVariant> > params,
                     bool need_authentication = false,
                     bool use_https = false);
  // Convenient function for extracting result from reply
  QVariantMap ExtractResult(QNetworkReply* reply);
  // Convenient function for extracting songs from grooveshark result
  SongList ExtractSongs(const QVariantMap& result);
  void ResetSessionId();


  GroovesharkUrlHandler* url_handler_;

  QString pending_search_;
  Playlist* pending_search_playlist_;

  int next_pending_search_id_;
  QMap<QNetworkReply*, int> pending_searches_;

  QMap<QNetworkReply*, PlaylistInfo> pending_retrieve_playlists_;

  QStandardItem* root_;
  QStandardItem* search_;

  NetworkAccessManager* network_;

  QMenu* context_menu_;
  QModelIndex context_item_;

  QTimer* search_delay_;
  QNetworkReply* last_search_reply_;

  QString username_;
  QString password_; // In fact, password's md5 hash
  QString user_id_;
  QString session_id_;
  QMap<QString, QVariant> country_;
  QByteArray api_key_;

  LoginState login_state_;
};


#endif // GROOVESHARKSERVICE_H
