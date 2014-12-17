/* This file is part of Clementine.
   Copyright 2014, Vlad Maltsev <shedwardx@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, Ivan Leontiev <leont.ivan@gmail.com>

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

#ifndef INTERNET_VKSERVICE_H_
#define INTERNET_VKSERVICE_H_

#include <memory>

#include "internetservice.h"
#include "internetmodel.h"
#include "core/song.h"

#include "vreen/audio.h"
#include "vreen/contact.h"

#include "vkconnection.h"
#include "vkurlhandler.h"

namespace Vreen {
class Client;
class Buddy;
}

class SearchBoxWidget;
class VkMusicCache;
class VkSearchDialog;

/***
 * Store information about user or group
 * using in bookmarks.
 */
class MusicOwner {
 public:
  MusicOwner() : songs_count_(0), id_(0) {}

  explicit MusicOwner(const QUrl& group_url);
  Song toOwnerRadio() const;

  QString name() const { return name_; }
  int id() const { return id_; }
  int song_count() const { return songs_count_; }
  static QList<MusicOwner> parseMusicOwnerList(const QVariant& request_result);
  // quick and dirty solution for creating MusicOwner instance for
  // logged in user
  void setId(int id) { id_ = id; }

 private:
  friend QDataStream& operator<<(QDataStream& stream, const MusicOwner& val);
  friend QDataStream& operator>>(QDataStream& stream, MusicOwner& val);
  friend QDebug operator<<(QDebug d, const MusicOwner& owner);

  int songs_count_;
  int id_;  // if id > 0 is user otherwise id group
  QString name_;
  // name used in url http://vk.com/<screen_name> for example:
  // http://vk.com/shedward
  QString screen_name_;
  QUrl photo_;
};

typedef QList<MusicOwner> MusicOwnerList;

Q_DECLARE_METATYPE(MusicOwner)

QDataStream& operator<<(QDataStream& stream, const MusicOwner& val);
QDataStream& operator>>(QDataStream& stream, MusicOwner& var);
QDebug operator<<(QDebug d, const MusicOwner& owner);

/***
 * The simple structure allows the handler to determine
 * how to react to the received request or quickly skip unwanted.
 */
struct SearchID {
  enum Type { GlobalSearch, LocalSearch, MoreLocalSearch, UserOrGroup };

  explicit SearchID(Type type) : type_(type) { id_ = last_id_++; }
  int id() const { return id_; }
  Type type() const { return type_; }

 private:
  static uint last_id_;
  int id_;
  Type type_;
};

/***
 * VkService
 */
class VkService : public InternetService {
  Q_OBJECT

 public:
  explicit VkService(Application* app, InternetModel* parent);
  ~VkService();

  static const char* kServiceName;
  static const char* kSettingGroup;
  static const char* kUrlScheme;
  static const char* kDefCacheFilename;
  static QString DefaultCacheDir();
  static const int kMaxVkSongList;
  static const int kMaxVkWallPostList;
  static const int kCustomSongCount;

  enum ItemType {
    Type_Loading = InternetModel::TypeCount,
    Type_More,

    Type_Recommendations,
    Type_Music,
    Type_Bookmark,
    Type_Album,
    Type_Wall,
    Type_AlbumList,

    Type_Search
  };

  enum Role {
    Role_MusicOwnerMetadata = InternetModel::RoleCount,
    Role_AlbumMetadata,
    Role_MoreMetadata
  };

  Application* app() const { return app_; }

  /* InternetService interface */
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  void ShowContextMenu(const QPoint& global_pos);
  void ItemDoubleClicked(QStandardItem* item);
  QList<QAction*> playlistitem_actions(const Song& song);

  /* Interface*/
  QWidget* HeaderWidget() const;

  /* Connection */
  void Login();
  void Logout();
  bool HasAccount() const;
  int UserID() const;
  void RequestUserProfile();
  bool WaitForReply(Vreen::Reply* reply);

  /* Music */
  void SongStarting(const Song& song);
  void SongStarting(const QUrl& url);  // Used if song taked from cache.
  void SongSkipped();
  UrlHandler::LoadResult GetSongResult(const QUrl& url);
  Vreen::AudioItem GetAudioItemFromUrl(const QUrl& url);
  // Return random song result from group playlist.
  UrlHandler::LoadResult GetGroupNextSongUrl(const QUrl& url);

  void SongSearch(SearchID id, const QString& query, int count = 50,
                  int offset = 0);
  void GroupSearch(SearchID id, const QString& query);

  /* Settings */
  void ReloadSettings();
  int maxGlobalSearch() const { return maxGlobalSearch_; }
  bool isCachingEnabled() const { return cachingEnabled_; }
  bool isGroupsInGlobalSearch() const { return groups_in_global_search_; }
  bool isBroadcasting() const { return enable_broadcast_; }
  QString cacheDir() const { return cacheDir_; }
  QString cacheFilename() const { return cacheFilename_; }
  bool isLoveAddToMyMusic() const { return love_is_add_to_mymusic_; }

 signals:
  void NameUpdated(const QString& name);
  void ConnectionStateChanged(Vreen::Client::State state);
  void LoginSuccess(bool success);
  void SongSearchResult(const SearchID& id, const SongList& songs);
  void GroupSearchResult(const SearchID& id, const MusicOwnerList& groups);
  void UserOrGroupSearchResult(const SearchID& id,
                               const MusicOwnerList& owners);
  void StopWaiting();

 public slots:
  void UpdateRoot();
  void ShowConfig();
  void FindUserOrGroup(const QString& q);

 private slots:
  /* Interface */
  void UpdateItem();

  /* Connection */
  void ChangeConnectionState(Vreen::Client::State state);
  void UserProfileRecived(const QVariant& result);
  void Error(Vreen::Client::Error error);

  /* Music */
  void SongStopped();
  void UpdateMusic(QStandardItem* item);
  void UpdateAlbumList(QStandardItem* item);
  void UpdateAlbumSongs(QStandardItem* item);
  void UpdateWallSongs(QStandardItem* item);
  void MoreWallSongs(QStandardItem* item);
  void FindSongs(const QString& query);
  void FindMore();
  void UpdateRecommendations();
  void MoreRecommendations();
  void FindThisArtist();
  void AddToMyMusic();
  void AddToMyMusicCurrent();
  void RemoveFromMyMusic();
  void AddSelectedToCache();
  void CopyShareUrl();
  void ShowSearchDialog();

  void AddSelectedToBookmarks();
  void RemoveFromBookmark();

  void SongSearchReceived(const SearchID& id, Vreen::AudioItemListReply* reply);
  void GroupSearchReceived(const SearchID& id, Vreen::Reply* reply);
  void UserOrGroupReceived(const SearchID& id, Vreen::Reply* reply);
  void AlbumListReceived(QStandardItem* parent,
                         Vreen::AudioAlbumItemListReply* reply);
  void BroadcastChangeReceived(Vreen::IntReply* reply);

  void AppendLoadedSongs(QStandardItem* item, Vreen::AudioItemListReply* reply);
  void RecommendationsLoaded(Vreen::AudioItemListReply* reply);
  void SearchResultLoaded(const SearchID& id, const SongList& songs);
  void WallPostsLoaded(QStandardItem* item, Vreen::Reply* reply, int offset);

 private:
  bool isItemBusy(const QStandardItem* item);
  int TypeOfItem(const QStandardItem* item);
  Vreen::AudioItemList handleWallPosts(const QVariant& response);

  /* Interface */
  QStandardItem* CreateAndAppendRow(QStandardItem* parent,
                                    VkService::ItemType type);
  void ClearStandardItem(QStandardItem* item);
  QStandardItem* GetBookmarkItemById(int id);
  void EnsureMenuCreated();

  /* Music */
  void LoadAndAppendSongList(QStandardItem* item, int uid, int album_id = -1);
  void LoadAndAppendWallSongList(QStandardItem* item, const MusicOwner& owner,
                                 int offset = 0);
  Song FromAudioItem(const Vreen::AudioItem& item);
  SongList FromAudioList(const Vreen::AudioItemList& list);
  void AppendSongs(QStandardItem* parent, const SongList& songs);

  QStandardItem* AppendBookmark(const MusicOwner& owner);
  void SaveBookmarks();
  void LoadBookmarks();

  void LoadAlbums(QStandardItem* parent, const MusicOwner& owner);
  QStandardItem* AppendAlbum(QStandardItem* parent,
                             const Vreen::AudioAlbumItem& album);
  QStandardItem* AppendAlbumList(QStandardItem* parent, bool myself = false);

  QStandardItem* AppendWall(QStandardItem* parent);
  QStandardItem* AppendMusic(QStandardItem* parent, bool myself = false);

  /* Interface */
  QStandardItem* root_item_;
  QStandardItem* recommendations_item_;
  QStandardItem* my_music_item_;
  QStandardItem* my_albums_item_;
  QStandardItem* search_result_item_;

  QMenu* context_menu_;

  QAction* update_item_;
  QAction* find_this_artist_;
  QAction* add_to_my_music_;
  QAction* remove_from_my_music_;
  QAction* add_song_to_cache_;
  QAction* copy_share_url_;
  QAction* add_to_bookmarks_;
  QAction* remove_from_bookmarks_;
  QAction* find_owner_;

  SearchBoxWidget* search_box_;
  VkSearchDialog* vk_search_dialog_;

  /* Connection */
  std::unique_ptr<Vreen::Client> client_;
  std::unique_ptr<VkConnection> connection_;
  VkUrlHandler* url_handler_;

  /* Music */
  std::unique_ptr<Vreen::AudioProvider> audio_provider_;
  VkMusicCache* cache_;
  // Keeping when more recent results recived.
  // Using for prevent loading tardy result instead.
  uint last_search_id_;
  QString last_query_;
  Song selected_song_;  // Store for context menu actions.
  Song current_song_;   // Store for actions with now playing song.
  // Store current group url for actions with it.
  QUrl current_group_url_;

  /* Settings */
  int maxGlobalSearch_;
  bool cachingEnabled_;
  bool love_is_add_to_mymusic_;
  bool groups_in_global_search_;
  bool enable_broadcast_;
  QString cacheDir_;
  QString cacheFilename_;
};

#endif  // INTERNET_VKSERVICE_H_
