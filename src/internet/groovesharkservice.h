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

#include <QSslError>

class GroovesharkUrlHandler;
class NetworkAccessManager;
class Playlist;
class SearchBoxWidget;

class QMenu;
class QNetworkReply;
class QNetworkRequest;
class QSortFilterProxyModel;

class GroovesharkService : public InternetService {
  Q_OBJECT
 public:
  GroovesharkService(Application* app, InternetModel *parent);
  ~GroovesharkService();

  enum Role {
    Role_UserPlaylistId = InternetModel::RoleCount,
    Role_PlaylistType
  };

  enum PlaylistType {
    UserPlaylist = Qt::UserRole,
    // Favorites and Library list are like playlists, but we want to do special
    // treatments in some cases
    UserFavorites,
    UserLibrary,
    SubscribedPlaylist
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
  smart_playlists::GeneratorPtr CreateGenerator(QStandardItem* item);
  void DropMimeData(const QMimeData* data, const QModelIndex& index);
  QList<QAction*> playlistitem_actions(const Song& song);
  void ShowContextMenu(const QPoint& global_pos);
  QWidget* HeaderWidget() const;

  // User should be logged in to be able to generate streaming urls
  QUrl GetStreamingUrlFromSongId(const QString& song_id, const QString& artist_id,
                                 QString* server_id, QString* stream_key,
                                 qint64* length_nanosec);
  void Login(const QString& username, const QString& password);
  void Logout();
  bool IsLoggedIn() const { return !session_id_.isEmpty(); }
  void RetrieveUserPlaylists();
  void RetrieveUserFavorites();
  void RetrieveUserLibrarySongs();
  void RetrievePopularSongs();
  void RetrievePopularSongsMonth();
  void RetrievePopularSongsToday();
  void RetrieveSubscribedPlaylists();
  void RetrieveAutoplayTags();
  void SetPlaylistSongs(int playlist_id, const QList<int>& songs_ids);
  void RemoveFromPlaylist(int playlist_id, const QList<int>& songs_ids_to_remove);
  // Refresh playlist_id playlist , or create it if it doesn't exist
  void RefreshPlaylist(int playlist_id);
  void DeletePlaylist(int playlist_id);
  void RenamePlaylist(int playlist_id);
  void AddUserFavoriteSong(int song_id);
  void RemoveFromFavorites(const QList<int>& songs_ids_to_remove);
  void AddUserLibrarySongs(const QList<int>& songs_ids);
  void RemoveFromLibrary(const QList<int>& songs_ids_to_remove);
  void GetSongUrlToShare(int song_id);
  void GetPlaylistUrlToShare(int playlist_id);
  // Start autoplay for the given tag_id, fill the autoplay_state, returns a
  // first song to play
  Song StartAutoplayTag(int tag_id, QVariantMap& autoplay_state);
  Song StartAutoplay(QVariantMap& autoplay_state);
  // Get another autoplay song. autoplay_state is the autoplay_state received from StartAutoplayTag
  Song GetAutoplaySong(QVariantMap& autoplay_state);
  void MarkStreamKeyOver30Secs(const QString& stream_key, const QString& server_id);
  void MarkSongComplete(const QString& song_id, const QString& stream_key, const QString& server_id);

  // Persisted in the settings and updated on each Login().
  LoginState login_state() const { return login_state_; }
  const QString& session_id() { return session_id_; }

  int SimpleSearch(const QString& query);
  int SearchAlbums(const QString& query);
  void GetAlbumSongs(quint64 album_id);

  static const char* kServiceName;
  static const char* kSettingsGroup;

 signals:
  void LoginFinished(bool success);
  void SimpleSearchResults(int id, SongList songs);
  // AlbumSearchResult emits the search id and the Grooveshark ids of the
  // albums found. Albums' songs will be loaded asynchronously and
  // AlbumSongsLoaded will be emitted, containing the actual Albums' songs.
  void AlbumSearchResult(int id, QList<quint64> albums_ids);
  void AlbumSongsLoaded(quint64 id, SongList songs);

 public slots:
  void Search(const QString& text, bool now = false);
  void ShowConfig();

 protected:
  struct PlaylistInfo {
    PlaylistInfo() {}
    PlaylistInfo(int id, QString name, QStandardItem* item = NULL)
      : id_(id), name_(name), item_(item) {}

    bool operator< (const PlaylistInfo other) const {
      return name_.localeAwareCompare(other.name_) < 0;
    }

    int id_;
    QString name_;
    QStandardItem* item_;
    QList<int> songs_ids_;
  };

 private slots:
  void SessionCreated(QNetworkReply* reply);
  void DoSearch();
  void SearchSongsFinished(QNetworkReply* reply);
  void SimpleSearchFinished(QNetworkReply* reply, int id);
  void SearchAlbumsFinished(QNetworkReply* reply, int id);
  void GetAlbumSongsFinished(QNetworkReply* reply, quint64 album_id);
  void Authenticated(QNetworkReply* reply);
  void UserPlaylistsRetrieved(QNetworkReply* reply);
  void UserFavoritesRetrieved(QNetworkReply* reply, int task_id);
  void UserLibrarySongsRetrieved(QNetworkReply* reply, int task_id);
  void PopularSongsMonthRetrieved(QNetworkReply* reply);
  void PopularSongsTodayRetrieved(QNetworkReply* reply);
  void SubscribedPlaylistsRetrieved(QNetworkReply* reply);
  void AutoplayTagsRetrieved(QNetworkReply* reply);
  void PlaylistSongsRetrieved(QNetworkReply* reply, int playlist_id);
  void PlaylistSongsSet(QNetworkReply* reply, int playlist_id, int task_id);
  void CreateNewPlaylist();
  void NewPlaylistCreated(QNetworkReply* reply, const QString& name);
  void DeleteCurrentPlaylist();
  void RenameCurrentPlaylist();
  void PlaylistDeleted(QNetworkReply* reply, int playlist_id);
  void PlaylistRenamed(QNetworkReply* reply, int playlist_id, const QString& new_name);
  void AddCurrentSongToUserFavorites()  { AddUserFavoriteSong(current_song_id_); }
  void AddCurrentSongToUserLibrary()    { AddUserLibrarySongs(QList<int>() << current_song_id_); }
  void AddCurrentSongToPlaylist(QAction* action);
  void UserFavoriteSongAdded(QNetworkReply* reply, int task_id);
  void UserLibrarySongAdded(QNetworkReply* reply, int task_id);
  void GetCurrentSongUrlToShare();
  void SongUrlToShareReceived(QNetworkReply* reply);
  void GetCurrentPlaylistUrlToShare();
  void PlaylistUrlToShareReceived(QNetworkReply* reply);
  void RemoveCurrentFromPlaylist();
  void RemoveCurrentFromFavorites();
  void RemoveCurrentFromLibrary();
  void SongsRemovedFromFavorites(QNetworkReply* reply, int task_id);
  void SongsRemovedFromLibrary(QNetworkReply* reply, int task_id);
  void StreamMarked(QNetworkReply* reply);
  void SongMarkedAsComplete(QNetworkReply* reply);

  void RequestSslErrors(const QList<QSslError>& errors);

  void Homepage();
  // Refresh all Grooveshark's items, and re-fill them
  void RefreshItems();

 private:
  void EnsureMenuCreated();
  void EnsureItemsCreated();
  void RemoveItems();
  void EnsureConnected();
  void ClearSearchResults();

  // Create a playlist item, with data set as excepted. Doesn't fill the item
  // with songs rows.
  QStandardItem* CreatePlaylistItem(const QString& playlist_name, int playlist_id);

  void AuthenticateSession();
  void InitCountry();

  // Create a request for the given method, with the given params.
  // If need_authentication is true, add session_id to params.
  // Returns the reply object created
  QNetworkReply* CreateRequest(
      const QString& method_name,
      const QList<QPair<QString, QVariant> >& params,
      bool use_https = false);
  // Convenient function which block until 'reply' replies, or timeout after 10
  // seconds. Returns false if reply has timeouted
  bool WaitForReply(QNetworkReply* reply);
  void ShowUrlBox(const QString& title, const QString& url);
  // Convenient function for extracting result from reply
  QVariantMap ExtractResult(QNetworkReply* reply);
  // Convenient function for extracting songs from grooveshark result. result
  // should be the "result" field of most Grooveshark replies
  SongList ExtractSongs(const QVariantMap& result);
  // Convenient function for extracting song from grooveshark result.
  // result_song should be the song field ('song', 'nextSong', ...) of the
  // Grooveshark reply
  Song ExtractSong(const QVariantMap& result_song);
  // Convenient functions for extracting Grooveshark songs ids
  QList<int> ExtractSongsIds(const QVariantMap& result);
  QList<int> ExtractSongsIds(const QList<QUrl>& urls);
  int ExtractSongId(const QUrl& url); // Returns 0 if url is not a Grooveshark url
  // Convenient function for extracting basic playlist info (only 'id' and
  // 'name': QStandardItem still need to be created), and sort them by name
  QList<PlaylistInfo> ExtractPlaylistInfo(const QVariantMap& result);

  void ResetSessionId();


  GroovesharkUrlHandler* url_handler_;

  QString pending_search_;

  int next_pending_search_id_;

  QSet<int> pending_retrieve_playlists_;

  QMap<int, PlaylistInfo> playlists_;
  QMap<int, PlaylistInfo> subscribed_playlists_;

  QStandardItem* root_;
  QStandardItem* search_;
  QStandardItem* popular_month_;
  QStandardItem* popular_today_;
  QStandardItem* stations_;
  QStandardItem* grooveshark_radio_;
  QStandardItem* favorites_;
  // Grooveshark Library (corresponds to Grooveshark 'MyMusic' actually, but
  // called 'Library' in the API).
  // Nothing to do with Clementine's local library
  QStandardItem* library_;
  QStandardItem* playlists_parent_;
  QStandardItem* subscribed_playlists_parent_;

  NetworkAccessManager* network_;

  QMenu* context_menu_;
  // IDs kept when showing menu, to know what the user has clicked on, to be
  // able to perform actions on corresponding items
  int current_song_id_;
  int current_playlist_id_;

  QAction* create_playlist_;
  QAction* delete_playlist_;
  QAction* rename_playlist_;
  QAction* remove_from_playlist_;
  QAction* remove_from_favorites_;
  QAction* remove_from_library_;
  QAction* get_url_to_share_song_;
  QAction* get_url_to_share_playlist_;
  QList<QAction*> playlistitem_actions_;

  SearchBoxWidget* search_box_;

  QTimer* search_delay_;
  QNetworkReply* last_search_reply_;

  QString username_;
  QString password_; // In fact, password's md5 hash
  QString user_id_;
  QString session_id_;
  QMap<QString, QVariant> country_;
  // The last artists and songs ids th users has listened to. Used for autoplay
  QList<int> last_artists_ids_;
  QList<int> last_songs_ids_;
  QByteArray api_key_;

  LoginState login_state_;

  // Tasks' ids: we need to keep them in mind to be able to update task status
  // on each step
  int task_popular_id_;
  int task_playlists_id_;
  int task_search_id_;

  static const char* kUrl;
  static const char* kUrlCover;
  static const char* kHomepage;

  static const int kSongSearchLimit;
  static const int kSongSimpleSearchLimit;
  static const int kAlbumSearchLimit;
  static const int kSearchDelayMsec;

  static const char* kApiKey;
  static const char* kApiSecret;
};


#endif // GROOVESHARKSERVICE_H
