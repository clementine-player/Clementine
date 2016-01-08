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

#include "vkservice.h"

#include <qmath.h>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#include <boost/scoped_ptr.hpp>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "ui/iconloader.h"
#include "widgets/didyoumean.h"

#include "globalsearch/globalsearch.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetplaylistitem.h"
#include "internet/core/searchboxwidget.h"

#include "vreen/audio.h"
#include "vreen/contact.h"
#include "vreen/roster.h"

#include "globalsearch/vksearchprovider.h"
#include "vkmusiccache.h"
#include "vksearchdialog.h"

const char* VkService::kServiceName = "Vk.com";
const char* VkService::kSettingGroup = "Vk.com";
const char* VkService::kUrlScheme = "vk";

const char* VkService::kDefCacheFilename = "%artist - %title";
const int VkService::kMaxVkSongList = 6000;
const int VkService::kMaxVkWallPostList = 100;
const int VkService::kMaxVkSongCount = 300;
const int VkService::kSearchDelayMsec = 400;

QString VkService::DefaultCacheDir() {
  return QDir::toNativeSeparators(
      Utilities::GetConfigPath(Utilities::Path_CacheRoot) + "/vkcache");
}

uint SearchID::last_id_ = 0;

/***
 * Little functions
 */

inline static void RemoveLastRow(QStandardItem* item,
                                 VkService::ItemType type) {
  QStandardItem* last_item = item->child(item->rowCount() - 1);
  if (last_item->data(InternetModel::Role_Type).toInt() == type) {
    item->removeRow(item->rowCount() - 1);
  } else {
    qLog(Error) << "Tryed to remove row" << last_item->text() << "of type"
                << last_item->data(InternetModel::Role_Type).toInt()
                << "instead of" << type << "from" << item->text();
  }
}

struct SongId {
  SongId() : audio_id(0), owner_id(0) {}

  int audio_id;
  int owner_id;
};

static SongId ExtractIds(const QUrl& url) {
  SongId res;
  QString song_ids = url.path().section('/', 1, 1);
  res.owner_id = song_ids.section('_', 0, 0).toInt();
  res.audio_id = song_ids.section('_', 1, 1).toInt();

  if (res.owner_id && res.audio_id && url.scheme() == "vk" &&
      url.host() == "song") {
    return res;
  } else {
    qLog(Error) << "Wromg song url" << url;
    return SongId();
  }
}

static Song SongFromUrl(const QUrl& url) {
  Song result;
  if (url.scheme() == "vk" && url.host() == "song") {
    QStringList ids = url.path().split('/');
    if (ids.size() == 4) {
      result.set_artist(ids[2]);
      result.set_title(ids[3]);
      result.set_valid(true);
    } else {
      qLog(Error) << "Wrong song url" << url;
      result.set_valid(false);
    }
    result.set_url(url);
  } else {
    qLog(Error) << "Wrong song url" << url;
    result.set_valid(false);
  }
  return result;
}

MusicOwner::MusicOwner(const QUrl& group_url) {
  QStringList tokens = group_url.path().split('/');
  if (group_url.scheme() == "vk" && group_url.host() == "group" &&
      tokens.size() == 5) {
    id_ = -tokens[1].toInt();
    songs_count_ = tokens[2].toInt();
    screen_name_ = tokens[3];
    name_ = tokens[4].replace('_', '/');
  } else {
    qLog(Error) << "Wrong group url" << group_url;
  }
}

Song MusicOwner::toOwnerRadio() const {
  Song song;
  song.set_title(QObject::tr("%1 (%2 songs)").arg(name_).arg(songs_count_));
  song.set_url(QUrl(QString("vk://group/%1/%2/%3/%4")
                        .arg(-id_)
                        .arg(songs_count_)
                        .arg(screen_name_)
                        .arg(QString(name_).replace('/', '_'))));
  song.set_artist(" " + QObject::tr("Community Radio"));
  song.set_valid(true);
  return song;
}

QDataStream& operator<<(QDataStream& stream, const MusicOwner& val) {
  stream << val.id_;
  stream << val.name_;
  stream << val.songs_count_;
  stream << val.screen_name_;
  return stream;
}

QDataStream& operator>>(QDataStream& stream, MusicOwner& var) {
  stream >> var.id_;
  stream >> var.name_;
  stream >> var.songs_count_;
  stream >> var.screen_name_;
  return stream;
}

QDebug operator<<(QDebug d, const MusicOwner& owner) {
  d << "MusicOwner(" << owner.id_ << "," << owner.name_ << ","
    << owner.songs_count_ << "," << owner.screen_name_ << ")";
  return d;
}

MusicOwnerList MusicOwner::parseMusicOwnerList(const QVariant& request_result) {
  auto list = request_result.toList();
  MusicOwnerList result;
  for (const auto& item : list) {
    auto map = item.toMap();
    MusicOwner owner;
    owner.songs_count_ = map.value("songs_count").toInt();
    owner.id_ = map.value("id").toInt();
    owner.name_ = map.value("name").toString();
    owner.screen_name_ = map.value("screen_name").toString();
    owner.photo_ = map.value("photo").toUrl();

    result.append(owner);
  }

  return result;
}

VkService::VkService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_item_(NULL),
      recommendations_item_(NULL),
      my_music_item_(NULL),
      my_albums_item_(NULL),
      search_result_item_(NULL),
      context_menu_(NULL),
      update_item_(NULL),
      find_this_artist_(NULL),
      add_to_my_music_(NULL),
      remove_from_my_music_(NULL),
      add_song_to_cache_(NULL),
      copy_share_url_(NULL),
      add_to_bookmarks_(NULL),
      remove_from_bookmarks_(NULL),
      find_owner_(NULL),
      search_box_(new SearchBoxWidget(this)),
      vk_search_dialog_(new VkSearchDialog(this)),
      client_(new Vreen::Client),
      connection_(new VkConnection(this)),
      url_handler_(new VkUrlHandler(this, this)),
      audio_provider_(new Vreen::AudioProvider(client_.get())),
      cache_(new VkMusicCache(app_, this)),
      last_search_id_(0),
      search_delay_(new QTimer(this)) {
  QSettings s;
  s.beginGroup(kSettingGroup);

  /* Init connection */
  client_->setTrackMessages(false);
  client_->setInvisible(true);
  client_->setConnection(connection_.get());

  ReloadSettings();

  if (HasAccount()) {
    Login();
  }

  connect(client_.get(), SIGNAL(connectionStateChanged(Vreen::Client::State)),
          SLOT(ChangeConnectionState(Vreen::Client::State)));
  connect(client_.get(), SIGNAL(error(Vreen::Client::Error)),
          SLOT(Error(Vreen::Client::Error)));

  /* Init interface */

  VkSearchProvider* search_provider = new VkSearchProvider(app_, this);
  search_provider->Init(this);
  app_->global_search()->AddProvider(search_provider);

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoLocalSearch()));

  connect(search_box_, SIGNAL(TextChanged(QString)), SLOT(FindSongs(QString)));
  connect(this, SIGNAL(SongSearchResult(SearchID, SongList)),
          SLOT(SearchResultLoaded(SearchID, SongList)));

  app_->player()->RegisterUrlHandler(url_handler_);
}

VkService::~VkService() {}

/***
 * Interface
 */

QStandardItem* VkService::CreateRootItem() {
  root_item_ = new QStandardItem(IconLoader::Load("vk", IconLoader::Provider), 
                                 kServiceName);
  root_item_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_item_;
}

void VkService::LazyPopulate(QStandardItem* parent) {
  switch (parent->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      UpdateRoot();
      break;
    case Type_Recommendations:
      UpdateRecommendations();
      break;
    case Type_AlbumList:
      UpdateAlbumList(parent);
      break;
    case Type_Music:
      UpdateMusic(parent);
      break;
    case Type_Album:
      UpdateAlbumSongs(parent);
      break;
    case Type_Wall:
      UpdateWallSongs(parent);
      break;
    default:
      break;
  }
}

void VkService::EnsureMenuCreated() {
  if (!context_menu_) {
    context_menu_ = new QMenu;

    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addSeparator();

    add_to_bookmarks_ =
        context_menu_->addAction(IconLoader::Load("list-add", IconLoader::Base), 
                                 tr("Add to bookmarks"), this, 
                                 SLOT(AddSelectedToBookmarks()));

    remove_from_bookmarks_ = context_menu_->addAction(
        IconLoader::Load("list-remove", IconLoader::Base), tr("Remove from bookmarks"), 
        this, SLOT(RemoveFromBookmark()));

    context_menu_->addSeparator();

    find_this_artist_ =
        context_menu_->addAction(IconLoader::Load("edit-find", IconLoader::Base), 
                                 tr("Find this artist"), this, 
                                 SLOT(FindThisArtist()));

    add_to_my_music_ =
        context_menu_->addAction(IconLoader::Load("list-add", IconLoader::Base), 
                                 tr("Add to My Music"), this, SLOT(AddToMyMusic()));

    remove_from_my_music_ = context_menu_->addAction(
        IconLoader::Load("list-remove", IconLoader::Base), tr("Remove from My Music"), 
        this, SLOT(RemoveFromMyMusic()));

    add_song_to_cache_ = context_menu_->addAction(IconLoader::Load("download", 
                                                  IconLoader::Base),
                                                  tr("Add song to cache"), this,
                                                  SLOT(AddSelectedToCache()));

    copy_share_url_ = context_menu_->addAction(
        IconLoader::Load("link", IconLoader::Base), tr("Copy share url to clipboard"), 
        this, SLOT(CopyShareUrl()));

    find_owner_ = context_menu_->addAction(IconLoader::Load("edit-find", 
                                           IconLoader::Base),
                                           tr("Add user/group to bookmarks"),
                                           this, SLOT(ShowSearchDialog()));

    update_item_ =
        context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base), 
                                 tr("Update"), this, SLOT(UpdateItem()));

    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                             tr("Configure Vk.com..."), this,
                             SLOT(ShowConfig()));
  }
}

void VkService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  QModelIndex current(model()->current_index());
  QStandardItem* current_item = model()->itemFromIndex(current);

  const int item_type = current.data(InternetModel::Role_Type).toInt();
  const int parent_type =
      current.parent().data(InternetModel::Role_Type).toInt();

  const bool is_playable = model()->IsPlayable(current);
  const bool is_my_music_item = current_item == my_music_item_ ||
                                current_item->parent() == my_music_item_;
  const bool is_track = item_type == InternetModel::Type_Track;

  const bool is_bookmark = item_type == Type_Bookmark;

  const bool is_updatable =
      item_type != Type_Bookmark && item_type != Type_Loading &&
      item_type != Type_More && item_type != Type_Search &&
      parent_type != Type_Search;

  const bool is_update_enable =
      // To prevent call LazyPopulate twice when we try to Update not populated
      // item
      !current.data(InternetModel::Role_CanLazyLoad).toBool() &&
      !current.parent().data(InternetModel::Role_CanLazyLoad).toBool() &&
      // disable update action until all of item's children have finished
      // updating
      !isItemBusy(model()->itemFromIndex(current));

  bool is_in_mymusic = false;
  bool is_cached = false;

  if (is_track) {
    selected_song_ =
        current.data(InternetModel::Role_SongMetadata).value<Song>();
    is_in_mymusic = is_my_music_item ||
                    ExtractIds(selected_song_.url()).owner_id == UserID();
    is_cached = cache_->InCache(selected_song_.url());
  }

  update_item_->setVisible(is_updatable);
  update_item_->setEnabled(is_update_enable);
  find_this_artist_->setVisible(is_track);
  add_song_to_cache_->setVisible(is_track && !is_cached);
  add_to_my_music_->setVisible(is_track && !is_in_mymusic);
  remove_from_my_music_->setVisible(is_track && is_in_mymusic);
  copy_share_url_->setVisible(is_track);
  remove_from_bookmarks_->setVisible(is_bookmark);
  add_to_bookmarks_->setVisible(false);

  GetAppendToPlaylistAction()->setEnabled(is_playable);
  GetReplacePlaylistAction()->setEnabled(is_playable);
  GetOpenInNewPlaylistAction()->setEnabled(is_playable);

  context_menu_->popup(global_pos);
}

void VkService::ItemDoubleClicked(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case Type_More:
      switch (item->parent()->data(InternetModel::Role_Type).toInt()) {
        case Type_Recommendations:
          MoreRecommendations();
          break;
        case Type_Search:
          FindMore();
          break;
        case Type_Wall:
          MoreWallSongs(item);
          break;
        default:
          qLog(Warning) << "Wrong parent for More item:"
                        << item->parent()->text();
      }
      break;
    default:
      qLog(Warning) << "Wrong item for double click with type:"
                    << item->data(InternetModel::Role_Type);
  }
}

QList<QAction*> VkService::playlistitem_actions(const Song& song) {
  EnsureMenuCreated();

  QList<QAction*> actions;

  if (song.url().host() == "song") {
    selected_song_ = song;
  } else if (song.url().host() == "group") {
    add_to_bookmarks_->setVisible(true);
    actions << add_to_bookmarks_;
    if (song.url() == current_group_url_) {
      // Selected now playing group.
      selected_song_ = current_song_;
    } else {
      // If selected not playing group, return only "Add to bookmarks" action.
      selected_song_ = song;
      return actions;
    }
  }

  // Adding songs actions.
  find_this_artist_->setVisible(true);
  actions << find_this_artist_;

  if (ExtractIds(selected_song_.url()).owner_id != UserID()) {
    add_to_my_music_->setVisible(true);
    actions << add_to_my_music_;
  } else {
    remove_from_my_music_->setVisible(true);
    actions << remove_from_my_music_;
  }

  copy_share_url_->setVisible(true);
  actions << copy_share_url_;

  if (!cache_->InCache(selected_song_.url())) {
    add_song_to_cache_->setVisible(true);
    actions << add_song_to_cache_;
  }

  return actions;
}

void VkService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Vk);
}

void VkService::UpdateRoot() {
  ClearStandardItem(root_item_);

  if (HasAccount()) {
    CreateAndAppendRow(root_item_, Type_Recommendations);
    AppendMusic(root_item_, true);
    AppendAlbumList(root_item_, true);
    LoadBookmarks();
  } else {
    ShowConfig();
  }
}

QWidget* VkService::HeaderWidget() const {
  if (HasAccount()) {
    return search_box_;
  } else {
    return NULL;
  }
}

QStandardItem* VkService::CreateAndAppendRow(QStandardItem* parent,
                                             VkService::ItemType type) {
  QStandardItem* item = NULL;

  switch (type) {
    case Type_Loading:
      item = new QStandardItem(tr("Loading..."));
      break;

    case Type_More:
      item = new QStandardItem(tr("More"));
      item->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                    InternetModel::Role_PlayBehaviour);
      break;

    case Type_Recommendations:
      item = new QStandardItem(IconLoader::Load("audio-headset", 
                               IconLoader::Base),
                               tr("My Recommendations"));
      item->setData(true, InternetModel::Role_CanLazyLoad);
      item->setData(InternetModel::PlayBehaviour_MultipleItems,
                    InternetModel::Role_PlayBehaviour);
      recommendations_item_ = item;
      break;

    case Type_Search:
      item = new QStandardItem(IconLoader::Load("edit-find", 
                               IconLoader::Base), tr("Search"));
      item->setData(InternetModel::PlayBehaviour_MultipleItems,
                    InternetModel::Role_PlayBehaviour);
      search_result_item_ = item;
      break;

    case Type_Bookmark:
      qLog(Error) << "Use AppendBookmark(const MusicOwner &owner)"
                  << "for creating Bookmark item instead.";
      break;

    case Type_Album:
      qLog(Error) << "Use AppendAlbum(const Vreen::AudioAlbumItem &album)"
                  << "for creating Album item instead.";
      break;

    default:
      qLog(Error) << "Invalid type for creating row: " << type;
      break;
  }

  item->setData(type, InternetModel::Role_Type);
  parent->appendRow(item);
  return item;
}

/***
 * Connection
 */

void VkService::Login() { client_->connectToHost(); }

void VkService::Logout() {
  if (connection_) {
    client_->disconnectFromHost();
    connection_->clear();
  }
}

bool VkService::HasAccount() const { return connection_->hasAccount(); }

int VkService::UserID() const { return connection_->uid(); }

void VkService::ChangeConnectionState(Vreen::Client::State state) {
  qLog(Debug) << "Connection state changed to" << state;
  switch (state) {
    case Vreen::Client::StateOnline:
      emit LoginSuccess(true);
      break;

    case Vreen::Client::StateInvalid:
    case Vreen::Client::StateOffline:
      emit LoginSuccess(false);
      break;
    case Vreen::Client::StateConnecting:
      return;
    default:
      qLog(Error) << "Wrong connection state " << state;
      return;
  }

  if (!root_item_->data(InternetModel::Role_CanLazyLoad).toBool()) {
    UpdateRoot();
  }
}

void VkService::RequestUserProfile() {
  QVariantMap args;
  args.insert("users_ids", "0");
  Vreen::Reply* reply = client_->request("users.get", args);

  connect(reply, SIGNAL(resultReady(QVariant)), this,
          SLOT(UserProfileRecived(QVariant)), Qt::UniqueConnection);
}

void VkService::UserProfileRecived(const QVariant& result) {
  auto list = result.toList();
  if (!list.isEmpty()) {
    auto profile = list[0].toMap();
    QString name = profile.value("first_name").toString() + " " +
                   profile.value("last_name").toString();
    emit NameUpdated(name);
  } else {
    qLog(Debug) << "Fetching user profile failed" << result;
  }
}

void VkService::Error(Vreen::Client::Error error) {
  QString msg;

  switch (error) {
    case Vreen::Client::ErrorApplicationDisabled:
      msg = "Application disabled";
      break;
    case Vreen::Client::ErrorIncorrectSignature:
      msg = "Incorrect signature";
      break;
    case Vreen::Client::ErrorAuthorizationFailed:
      msg = "Authorization failed";
      emit LoginSuccess(false);
      break;
    case Vreen::Client::ErrorToManyRequests:
      msg = "Too many requests";
      break;
    case Vreen::Client::ErrorPermissionDenied:
      msg = "Permission denied";
      break;
    case Vreen::Client::ErrorCaptchaNeeded:
      msg = "Captcha needed";
      QMessageBox::critical(NULL, tr("Error"),
                            tr("Captcha is needed.\n"
                               "Try to login into Vk.com with your browser,"
                               "to fix this problem."),
                            QMessageBox::Close);
      break;
    case Vreen::Client::ErrorMissingOrInvalidParameter:
      msg = "Missing or invalid parameter";
      break;
    case Vreen::Client::ErrorNetworkReply:
      msg = "Network reply";
      break;
    default:
      msg = "Unknown error";
      break;
  }

  qLog(Error) << "Client error: " << error << msg;
}

/***
 * My Music
 */

void VkService::UpdateMusic(QStandardItem* item) {
  if (item) {
    MusicOwner owner = item->data(Role_MusicOwnerMetadata).value<MusicOwner>();
    LoadAndAppendSongList(item, owner.id());
  }
}

/***
 * Recommendation
 */

void VkService::UpdateRecommendations() {
  ClearStandardItem(recommendations_item_);
  CreateAndAppendRow(recommendations_item_, Type_Loading);

  auto my_audio =
      audio_provider_->getRecommendationsForUser(0, kMaxVkSongCount, 0);

  NewClosure(my_audio, SIGNAL(resultReady(QVariant)), this,
             SLOT(RecommendationsLoaded(Vreen::AudioItemListReply*)), my_audio);
}

void VkService::MoreRecommendations() {
  RemoveLastRow(recommendations_item_, Type_More);
  CreateAndAppendRow(recommendations_item_, Type_Loading);

  auto my_audio = audio_provider_->getRecommendationsForUser(
      0, kMaxVkSongCount, recommendations_item_->rowCount() - 1);

  NewClosure(my_audio, SIGNAL(resultReady(QVariant)), this,
             SLOT(RecommendationsLoaded(Vreen::AudioItemListReply*)), my_audio);
}

void VkService::RecommendationsLoaded(Vreen::AudioItemListReply* reply) {
  SongList songs = FromAudioList(reply->result());
  RemoveLastRow(recommendations_item_, Type_Loading);
  AppendSongs(recommendations_item_, songs);
  if (songs.count() > 0) {
    CreateAndAppendRow(recommendations_item_, Type_More);
  }
}

/***
 * Bookmarks
 */

void VkService::AddSelectedToBookmarks() {
  QUrl group_url;
  if (selected_song_.url().scheme() == "vk" &&
      selected_song_.url().host() == "song") {
    // Selected song is song of now playing group, so group url in
    // current_group_url_
    group_url = current_group_url_;
  } else {
    // Otherwise selectet group radio in playlist
    group_url = selected_song_.url();
  }

  AppendBookmark(MusicOwner(group_url));
  SaveBookmarks();
}

void VkService::RemoveFromBookmark() {
  QModelIndex current(model()->current_index());
  root_item_->removeRow(current.row());
  SaveBookmarks();
}

void VkService::SaveBookmarks() {
  QSettings s;
  s.beginGroup(kSettingGroup);

  s.beginWriteArray("bookmarks");
  int index = 0;
  for (int i = 0; i < root_item_->rowCount(); ++i) {
    auto item = root_item_->child(i);
    if (item->data(InternetModel::Role_Type).toInt() == Type_Bookmark) {
      MusicOwner owner =
          item->data(Role_MusicOwnerMetadata).value<MusicOwner>();
      s.setArrayIndex(index);
      qLog(Info) << "Save" << index << ":" << owner;
      s.setValue("owner", QVariant::fromValue(owner));
      ++index;
    }
  }
  s.endArray();
}

void VkService::LoadBookmarks() {
  QSettings s;
  s.beginGroup(kSettingGroup);

  int max = s.beginReadArray("bookmarks");
  for (int i = 0; i < max; ++i) {
    s.setArrayIndex(i);
    MusicOwner owner = s.value("owner").value<MusicOwner>();
    qLog(Info) << "Load" << i << ":" << owner;
    AppendBookmark(owner);
  }
  s.endArray();
}

QStandardItem* VkService::AppendBookmark(const MusicOwner& owner) {
  QIcon icon;
  if (owner.id() > 0) {
    icon = IconLoader::Load("x-clementine-artist", IconLoader::Base);
  } else {
    icon = IconLoader::Load("group", IconLoader::Base);
  }
  QStandardItem* item = new QStandardItem(icon, owner.name());

  item->setData(QVariant::fromValue(owner), Role_MusicOwnerMetadata);
  item->setData(Type_Bookmark, InternetModel::Role_Type);

  AppendWall(item);
  AppendMusic(item);
  AppendAlbumList(item);

  root_item_->appendRow(item);
  return item;
}

void VkService::UpdateItem() {
  QModelIndex current(model()->current_index());
  QStandardItem* item = current.data(InternetModel::Role_Type).toInt() ==
                                InternetModel::Type_Track
                            ? model()->itemFromIndex(current.parent())
                            : model()->itemFromIndex(current);

  LazyPopulate(item);
}

void VkService::UpdateAlbumList(QStandardItem* item) {
  MusicOwner owner = item->data(Role_MusicOwnerMetadata).value<MusicOwner>();
  ClearStandardItem(item);
  CreateAndAppendRow(item, Type_Loading);
  LoadAlbums(item, owner);
}

/***
 * Albums
 */

void VkService::LoadAlbums(QStandardItem* parent, const MusicOwner& owner) {
  auto albums_request = audio_provider_->getAlbums(owner.id());
  NewClosure(
      albums_request, SIGNAL(resultReady(QVariant)), this,
      SLOT(AlbumListReceived(QStandardItem*, Vreen::AudioAlbumItemListReply*)),
      parent, albums_request);
}

QStandardItem* VkService::AppendAlbum(QStandardItem* parent,
                                      const Vreen::AudioAlbumItem& album) {
  QStandardItem* item =
      new QStandardItem(IconLoader::Load("view-media-playlist", IconLoader::Base), 
                        album.title());

  item->setData(QVariant::fromValue(album), Role_AlbumMetadata);
  item->setData(Type_Album, InternetModel::Role_Type);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::PlayBehaviour_MultipleItems,
                InternetModel::Role_PlayBehaviour);

  parent->appendRow(item);
  return item;
}

QStandardItem* VkService::AppendAlbumList(QStandardItem* parent, bool myself) {
  MusicOwner owner;
  QStandardItem* item;

  if (myself) {
    item = new QStandardItem(IconLoader::Load("x-clementine-album", 
                             IconLoader::Base), tr("My Albums"));
    // TODO(Ivan Leontiev): Do this better. We have incomplete MusicOwner
    // instance for logged in user.
    owner.setId(UserID());
    my_albums_item_ = item;
  } else {
    owner = parent->data(Role_MusicOwnerMetadata).value<MusicOwner>();
    item = new QStandardItem(IconLoader::Load("x-clementine-album", 
                             IconLoader::Base), tr("Albums"));
  }

  item->setData(QVariant::fromValue(owner), Role_MusicOwnerMetadata);
  item->setData(Type_AlbumList, InternetModel::Role_Type);
  item->setData(true, InternetModel::Role_CanLazyLoad);

  parent->appendRow(item);
  return item;
}

void VkService::AlbumListReceived(QStandardItem* parent,
                                  Vreen::AudioAlbumItemListReply* reply) {
  Vreen::AudioAlbumItemList albums = reply->result();
  RemoveLastRow(parent, Type_Loading);
  for (const auto& album : albums) {
    AppendAlbum(parent, album);
  }
}

void VkService::UpdateAlbumSongs(QStandardItem* item) {
  Vreen::AudioAlbumItem album =
      item->data(Role_AlbumMetadata).value<Vreen::AudioAlbumItem>();

  LoadAndAppendSongList(item, album.ownerId(), album.id());
}

/***
 * Wall
 */

QStandardItem* VkService::AppendWall(QStandardItem* parent) {
  QStandardItem* item =
      new QStandardItem(IconLoader::Load("view-media-playlist", 
                        IconLoader::Base), tr("Wall"));
  MusicOwner owner = parent->data(Role_MusicOwnerMetadata).value<MusicOwner>();

  item->setData(QVariant::fromValue(owner), Role_MusicOwnerMetadata);
  item->setData(Type_Wall, InternetModel::Role_Type);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::PlayBehaviour_MultipleItems,
                InternetModel::Role_PlayBehaviour);

  parent->appendRow(item);
  return item;
}

QStandardItem* VkService::AppendMusic(QStandardItem* parent, bool myself) {
  MusicOwner owner;
  QStandardItem* item;

  if (myself) {
    item = new QStandardItem(IconLoader::Load("love", IconLoader::Lastfm), 
                             tr("My Music"));
    // TODO(Ivan Leontiev): Do this better. We have incomplete MusicOwner
    // instance for logged in user.
    owner.setId(UserID());
    my_music_item_ = item;
  } else {
    item = new QStandardItem(IconLoader::Load("view-media-playlist", 
                             IconLoader::Base), tr("Music"));
    owner = parent->data(Role_MusicOwnerMetadata).value<MusicOwner>();
  }

  item->setData(QVariant::fromValue(owner), Role_MusicOwnerMetadata);
  item->setData(Type_Music, InternetModel::Role_Type);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::PlayBehaviour_MultipleItems,
                InternetModel::Role_PlayBehaviour);

  parent->appendRow(item);
  return item;
}

void VkService::UpdateWallSongs(QStandardItem* item) {
  MusicOwner owner = item->data(Role_MusicOwnerMetadata).value<MusicOwner>();
  ClearStandardItem(item);
  LoadAndAppendWallSongList(item, owner);
}

void VkService::MoreWallSongs(QStandardItem* item) {
  QStandardItem* parent = item->parent();
  MusicOwner owner = parent->data(Role_MusicOwnerMetadata).value<MusicOwner>();
  int offset = item->data(Role_MoreMetadata).value<int>();

  RemoveLastRow(parent, Type_More);
  LoadAndAppendWallSongList(parent, owner, offset);
}

void VkService::WallPostsLoaded(QStandardItem* item, Vreen::Reply* reply,
                                int offset) {
  auto response = reply->response().toMap();
  int count = response.value("count").toInt();

  SongList songs = FromAudioList(handleWallPosts(response.value("items")));

  RemoveLastRow(item, Type_Loading);
  AppendSongs(item, songs);
  if (count > offset) {
    auto m = CreateAndAppendRow(item, Type_More);
    m->setData(offset, Role_MoreMetadata);
  }
}

void VkService::LoadAndAppendWallSongList(QStandardItem* item,
                                          const MusicOwner& owner, int offset) {
  if (item) {
    CreateAndAppendRow(item, Type_Loading);
    QVariantMap args;
    QString vk_script =
        "var a = API.wall.get({"
        "   \"owner_id\": Args.q,"
        "   \"count\": Args.count,"
        "   \"offset\": Args.offset"
        "});"
        "return {\"count\": a.count, \"items\": a.items@.attachments};";

    args.insert("v", "5.25");
    args.insert("q", owner.id());
    args.insert("offset", offset);
    args.insert("count", kMaxVkWallPostList);
    args.insert("code", vk_script);

    auto reply = client_->request("execute", args);
    NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
               SLOT(WallPostsLoaded(QStandardItem*, Vreen::Reply*, int)), item,
               reply, offset + kMaxVkWallPostList);
  }
}

/***
 * Features
 */

void VkService::FindThisArtist() {
  search_box_->SetText(selected_song_.artist());
}

void VkService::AddToMyMusic() {
  SongId id = ExtractIds(selected_song_.url());
  auto reply = audio_provider_->addToLibrary(id.audio_id, id.owner_id);
  NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
             SLOT(UpdateMusic(QStandardItem*)), my_music_item_);
}

void VkService::AddToMyMusicCurrent() {
  if (isLoveAddToMyMusic() && current_song_.is_valid()) {
    selected_song_ = current_song_;
    AddToMyMusic();
  }
}

void VkService::RemoveFromMyMusic() {
  SongId id = ExtractIds(selected_song_.url());
  if (id.owner_id == UserID()) {
    auto reply = audio_provider_->removeFromLibrary(id.audio_id, id.owner_id);
    NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
               SLOT(UpdateMusic(QStandardItem*)), my_music_item_);
  } else {
    qLog(Error) << "Tried to delete song that not owned by user (" << UserID()
                << selected_song_.url();
  }
}

void VkService::AddSelectedToCache() {
  QUrl selected_song_media_url =
      GetAudioItemFromUrl(selected_song_.url()).url();
  cache_->AddToCache(selected_song_.url(), selected_song_media_url, true);
}

void VkService::CopyShareUrl() {
  QByteArray share_url("http://vk.com/audio?q=");
  share_url += QUrl::toPercentEncoding(
      QString(selected_song_.artist() + " " + selected_song_.title()));

  QApplication::clipboard()->setText(share_url);
}

/***
 * Search
 */

void VkService::DoLocalSearch() {
  ClearStandardItem(search_result_item_);
  CreateAndAppendRow(search_result_item_, Type_Loading);
  SearchID id(SearchID::LocalSearch);

  last_search_id_ = id.id();
  SongSearch(id, last_query_);
}

void VkService::FindSongs(const QString& query) {
  last_query_ = query;

  if (query.isEmpty()) {
    search_delay_->stop();
    root_item_->removeRow(search_result_item_->row());
    search_result_item_ = NULL;
    last_search_id_ = 0;
    return;
  }

  search_delay_->start();

  if (!search_result_item_) {
    CreateAndAppendRow(root_item_, Type_Search);
  }
}

void VkService::FindMore() {
  RemoveLastRow(search_result_item_, Type_More);
  CreateAndAppendRow(search_result_item_, Type_Loading);
  SearchID id(SearchID::MoreLocalSearch);

  last_search_id_ = id.id();
  SongSearch(id, last_query_, kMaxVkSongCount,
             search_result_item_->rowCount() - 1);
}

void VkService::SearchResultLoaded(const SearchID& id, const SongList& songs) {
  if (!search_result_item_) {
    return;  // Result received when search is already over.
  }

  if (id.id() == last_search_id_) {
    if (id.type() == SearchID::LocalSearch) {
      ClearStandardItem(search_result_item_);
    } else if (id.type() == SearchID::MoreLocalSearch) {
      RemoveLastRow(search_result_item_, Type_Loading);
    } else {
      return;  // Others request types ignored.
    }

    if (!songs.isEmpty()) {
      AppendSongs(search_result_item_, songs);
      CreateAndAppendRow(search_result_item_, Type_More);
    }

    // If new search, scroll to search results.
    if (id.type() == SearchID::LocalSearch) {
      QModelIndex index =
          model()->merged_model()->mapFromSource(search_result_item_->index());
      ScrollToIndex(index);
    }
  }
}

/***
 * Load song list methods
 */

void VkService::LoadAndAppendSongList(QStandardItem* item, int uid,
                                      int album_id) {
  if (item) {
    ClearStandardItem(item);
    CreateAndAppendRow(item, Type_Loading);
    auto audioreq =
        audio_provider_->getContactAudio(uid, kMaxVkSongList, 0, album_id);
    NewClosure(
        audioreq, SIGNAL(resultReady(QVariant)), this,
        SLOT(AppendLoadedSongs(QStandardItem*, Vreen::AudioItemListReply*)),
        item, audioreq);
  }
}

void VkService::AppendLoadedSongs(QStandardItem* item,
                                  Vreen::AudioItemListReply* reply) {
  SongList songs = FromAudioList(reply->result());

  if (item) {
    ClearStandardItem(item);
    if (songs.count() > 0) {
      AppendSongs(item, songs);
      return;
    }
  } else {
    qLog(Warning) << "Item for request not exist";
  }

  item->appendRow(new QStandardItem(
      tr("Connection trouble "
         "or audio is disabled by owner")));
}

static QString SanitiseCharacters(QString str) {
  // Remove all leading and trailing unicode symbols
  // that some users love to add to title and artist.
  str = str.remove(QRegExp("^[^\\w]*"));
  str = str.remove(QRegExp("[^])\\w]*$"));
  return str;
}

Song VkService::FromAudioItem(const Vreen::AudioItem& item) {
  Song song;
  song.set_title(SanitiseCharacters(item.title()));
  song.set_artist(SanitiseCharacters(item.artist()));
  song.set_length_nanosec(qFloor(item.duration()) * kNsecPerSec);

  QString url = QString("vk://song/%1_%2/%3/%4")
                    .arg(item.ownerId())
                    .arg(item.id())
                    .arg(item.artist().replace('/', '_'))
                    .arg(item.title().replace('/', '_'));

  song.set_url(QUrl(url));
  song.set_valid(true);
  return song;
}

SongList VkService::FromAudioList(const Vreen::AudioItemList& list) {
  SongList song_list;
  for (const Vreen::AudioItem& item : list) {
    song_list.append(FromAudioItem(item));
  }
  return song_list;
}

/***
 * Url handling
 */

Vreen::AudioItem VkService::GetAudioItemFromUrl(const QUrl& url) {
  QStringList tokens = url.path().split('/');

  if (tokens.count() < 2) {
    qLog(Error) << "Wrong song url" << url;
    return Vreen::AudioItem();
  }

  QString song_id = tokens[1];

  if (HasAccount()) {
    Vreen::AudioItemListReply* song_request =
        audio_provider_->getAudiosByIds(song_id);
    emit StopWaiting();  // Stop all previous requests.
    bool success = WaitForReply(song_request);

    if (success && !song_request->result().isEmpty()) {
      return song_request->result()[0];
    }
  }

  qLog(Info) << "Unresolved url by id" << song_id;
  return Vreen::AudioItem();
}

UrlHandler::LoadResult VkService::GetSongResult(const QUrl& url) {
  // Try get from cache
  QUrl media_url = cache_->Get(url);
  if (media_url.isValid()) {
    SongStarting(url);
    return UrlHandler::LoadResult(url, UrlHandler::LoadResult::TrackAvailable,
                                  media_url);
  }

  // Otherwise get fresh link
  auto audio_item = GetAudioItemFromUrl(url);
  media_url = audio_item.url();
  if (media_url.isValid()) {
    Song song = FromAudioItem(audio_item);
    SongStarting(song);

    if (cachingEnabled_) {
      cache_->AddToCache(url, media_url);
    }

    return UrlHandler::LoadResult(url, UrlHandler::LoadResult::TrackAvailable,
                                  media_url, song.length_nanosec());
  }

  return UrlHandler::LoadResult(url);
}

UrlHandler::LoadResult VkService::GetGroupNextSongUrl(const QUrl& url) {
  QStringList tokens = url.path().split('/');
  if (tokens.count() < 3) {
    qLog(Error) << "Wrong url" << url;
    return UrlHandler::LoadResult(url);
  }

  int gid = tokens[1].toInt();
  int songs_count = tokens[2].toInt();

  if (songs_count > kMaxVkSongList) {
    songs_count = kMaxVkSongList;
  }

  if (HasAccount()) {
    // Getting one random song from groups playlist.
    Vreen::AudioItemListReply* song_request =
        audio_provider_->getContactAudio(-gid, 1, qrand() % songs_count);

    emit StopWaiting();  // Stop all previous requests.
    bool success = WaitForReply(song_request);

    if (success && !song_request->result().isEmpty()) {
      Vreen::AudioItem song = song_request->result()[0];
      current_group_url_ = url;
      SongStarting(FromAudioItem(song));
      emit StreamMetadataFound(url, current_song_);
      return UrlHandler::LoadResult(url, UrlHandler::LoadResult::TrackAvailable,
                                    song.url(), current_song_.length_nanosec());
    }
  }

  qLog(Info) << "Unresolved group url" << url;
  return UrlHandler::LoadResult(url);
}

/***
 * Song playing
 */

void VkService::SongStarting(const QUrl& url) {
  SongStarting(SongFromUrl(url));
}

void VkService::SongStarting(const Song& song) {
  current_song_ = song;

  if (isBroadcasting() && HasAccount()) {
    auto id = ExtractIds(song.url());
    auto reply =
        audio_provider_->setBroadcast(id.audio_id, id.owner_id, IdList());
    NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
               SLOT(BroadcastChangeReceived(Vreen::IntReply*)), reply);
    connect(app_->player(), SIGNAL(Stopped()), this, SLOT(SongStopped()),
            Qt::UniqueConnection);
    qLog(Debug) << "Broadcasting" << song.artist() << "-" << song.title();
  }
}

void VkService::SongSkipped() {
  current_song_.set_valid(false);
  cache_->BreakCurrentCaching();
}

void VkService::SongStopped() {
  current_song_.set_valid(false);

  if (isBroadcasting() && HasAccount()) {
    auto reply = audio_provider_->resetBroadcast(IdList());
    NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
               SLOT(BroadcastChangeReceived(Vreen::IntReply*)), reply);
    disconnect(app_->player(), SIGNAL(Stopped()), this, SLOT(SongStopped()));
    qLog(Debug) << "End of broadcasting";
  }
}

void VkService::BroadcastChangeReceived(Vreen::IntReply* reply) {
  qLog(Debug) << "Broadcast changed for " << reply->result();
}

/***
 * Search
 */

void VkService::SongSearch(SearchID id, const QString& query, int count,
                           int offset) {
  auto reply = audio_provider_->searchAudio(
      query, count, offset, false, Vreen::AudioProvider::SortByPopularity);
  NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
             SLOT(SongSearchReceived(SearchID, Vreen::AudioItemListReply*)), id,
             reply);
}

void VkService::SongSearchReceived(const SearchID& id,
                                   Vreen::AudioItemListReply* reply) {
  SongList songs = FromAudioList(reply->result());
  emit SongSearchResult(id, songs);
}

void VkService::GroupSearch(SearchID id, const QString& query) {
  QVariantMap args;
  args.insert("q", query);

  //    This is using of 'execute' method that execute
  //    this VKScript method on vk server:
  /*
      var groups = API.groups.search({"q": Args.q});
      if (groups.length == 0) {
          return [];
      }

      var i = 1;
      var res = [];
      while (i < groups.length - 1) {
          i = i + 1;
          var grp = groups[i];
          var songs = API.audio.getCount({oid: -grp.gid});
          if ( songs > 1 &&
              (grp.is_closed == 0 || grp.is_member == 1))
          {
              res = res + [{"songs_count" : songs,
                              "id" : -grp.gid,
                              "name" : grp.name,
                              "screen_name" : grp.screen_name,
                              "photo": grp.photo}];
          }
      }
      return  res;
  */
  //
  //    I leave it here in case if my vk app disappear or smth.

  auto reply = client_->request("execute.searchMusicGroup", args);

  NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
             SLOT(GroupSearchReceived(SearchID, Vreen::Reply*)), id, reply);
}

void VkService::GroupSearchReceived(const SearchID& id, Vreen::Reply* reply) {
  QVariant groups = reply->response();
  emit GroupSearchResult(id, MusicOwner::parseMusicOwnerList(groups));
}

/***
 * Vk search user or group.
 */

void VkService::ShowSearchDialog() {
  if (vk_search_dialog_->exec() == QDialog::Accepted) {
    AppendBookmark(vk_search_dialog_->found());
    SaveBookmarks();
  }
}

void VkService::FindUserOrGroup(const QString& q) {
  QVariantMap args;
  args.insert("q", q);

  //    This is using of 'execute' method that execute
  //    this VKScript method on vk server:
  /*
      var q = Args.q;
      if (q + "" == ""){
          return [];
      }

      var results_count = 0;
      var res = [];

      // Search groups
      var groups = API.groups.search({"q": q});

      var i = 0;
      while (i < groups.length - 1 && results_count <= 5) {
          i = i + 1;
          var grp = groups[i];
          var songs = API.audio.getCount({oid: -grp.gid});
          // Add only accessible groups with songs
          if ( songs > 1 &&
              (grp.is_closed == 0 || grp.is_member == 1))
          {
              results_count = results_count + 1;
              res = res + [{"songs_count" : songs,
                              "id" : -grp.gid,
                              "name" : grp.name,
                              "screen_name" : grp.screen_name,
                              "photo": grp.photo}];
          }
      }

      // Search peoples
      var peoples = API.users.search({"q": q,
                                      "count":10,
                                      "fields":"screen_name,photo"});
      var i = 0;
      while (i < peoples.length - 1 && results_count <= 7) {
          i = i + 1;
          var user = peoples[i];
          var songs = API.audio.getCount({"oid": user.uid});
          // Add groups only with songs
          if (songs > 1) {
              results_count = results_count + 1;
              res = res + [{"songs_count" : songs,
                            "id" : user.uid,
                            "name" : user.first_name + " " + user.last_name,
                            "screen_name" : user.screen_name,
                            "phone" : user.photo}];
          }
      }

      return  res;
  */
  //    I leave it here just in case if my vk app will disappear or smth.

  auto reply = client_->request("execute.searchMusicOwner", args);

  NewClosure(reply, SIGNAL(resultReady(QVariant)), this,
             SLOT(UserOrGroupReceived(SearchID, Vreen::Reply*)),
             SearchID(SearchID::UserOrGroup), reply);
}

void VkService::UserOrGroupReceived(const SearchID& id, Vreen::Reply* reply) {
  QVariant owners = reply->response();
  emit UserOrGroupSearchResult(id, MusicOwner::parseMusicOwnerList(owners));
}

/***
 * Utils
 */

int VkService::TypeOfItem(const QStandardItem* item) {
  return item->data(InternetModel::Role_Type).toInt();
}

bool VkService::isItemBusy(const QStandardItem* item) {
  const QStandardItem* cur_item =
      TypeOfItem(item) == InternetModel::Type_Track ? item->parent() : item;

  int r_count = cur_item->rowCount();
  bool flag = false;

  if (r_count) {
    if (TypeOfItem(cur_item->child(r_count - 1)) == Type_Loading) return true;

    int t = TypeOfItem(cur_item);
    if (cur_item == root_item_ || t == Type_Bookmark || t == Type_AlbumList) {
      for (int i = 0; i < r_count; i++) {
        flag |= isItemBusy(cur_item->child(i));
      }
    }
  }
  return flag;
}

void VkService::AppendSongs(QStandardItem* parent, const SongList& songs) {
  for (const auto& song : songs) {
    parent->appendRow(CreateSongItem(song));
  }
}

void VkService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingGroup);
  maxGlobalSearch_ = s.value("max_global_search", kMaxVkSongCount).toInt();
  cachingEnabled_ = s.value("cache_enabled", false).toBool();
  cacheDir_ = s.value("cache_dir", DefaultCacheDir()).toString();
  cacheFilename_ = s.value("cache_filename", kDefCacheFilename).toString();
  love_is_add_to_mymusic_ = s.value("love_is_add_to_my_music", false).toBool();
  groups_in_global_search_ = s.value("groups_in_global_search", false).toBool();

  if (!s.contains("enable_broadcast")) {
    // Need to update premissions
    Logout();
  }
  enable_broadcast_ = s.value("enable_broadcast", false).toBool();
}

void VkService::ClearStandardItem(QStandardItem* item) {
  if (item && item->hasChildren()) {
    item->removeRows(0, item->rowCount());
  }
}

bool VkService::WaitForReply(Vreen::Reply* reply) {
  QEventLoop event_loop;
  QTimer timeout_timer;
  connect(this, SIGNAL(StopWaiting()), &timeout_timer, SLOT(stop()));
  connect(&timeout_timer, SIGNAL(timeout()), &event_loop, SLOT(quit()));
  connect(reply, SIGNAL(resultReady(QVariant)), &event_loop, SLOT(quit()));
  timeout_timer.start(10000);
  event_loop.exec();
  if (!timeout_timer.isActive()) {
    qLog(Error) << "Vk.com request timeout";
    return false;
  }
  timeout_timer.stop();
  return true;
}

Vreen::AudioItemList VkService::handleWallPosts(const QVariant& response) {
  Vreen::AudioItemList items;
  auto list = response.toList();
  for (const auto& i : list) {
    auto attachments = i.toList();
    for (const auto& j : attachments) {
      auto item = j.toMap();
      if (item.value("type") == "audio") {
        auto map = item.value("audio").toMap();
        Vreen::AudioItem audio;

        audio.setId(map.value("id").toInt());
        audio.setOwnerId(map.value("owner_id").toInt());
        audio.setArtist(map.value("artist").toString());
        audio.setTitle(map.value("title").toString());
        audio.setDuration(map.value("duration").toReal());
        audio.setAlbumId(map.value("album").toInt());
        audio.setLyricsId(map.value("lyrics_id").toInt());
        audio.setUrl(map.value("url").toUrl());

        items.append(audio);
      }
    }
  }
  return items;
}
