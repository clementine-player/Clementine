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

#include "groovesharkservice.h"

#include <boost/scoped_ptr.hpp>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "qtiocompressor.h"

#include "internetmodel.h"
#include "groovesharkradio.h"
#include "groovesharkurlhandler.h"
#include "searchboxwidget.h"

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/scopedtransaction.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/groovesharksearchprovider.h"
#include "playlist/playlist.h"
#include "playlist/playlistcontainer.h"
#include "playlist/playlistmanager.h"
#include "ui/iconloader.h"

using smart_playlists::Generator;
using smart_playlists::GeneratorPtr;

// The Grooveshark terms of service require that application keys are not
// accessible to third parties. Therefore this application key is obfuscated to
// prevent third parties from viewing it.
const char* GroovesharkService::kApiKey = "clementineplayer";
const char* GroovesharkService::kApiSecret = "OzLDTB5XqmhkkhxMUK0/Mp5PQgD5O27DTEJa/jtkwEw=";

const char* GroovesharkService::kServiceName = "Grooveshark";
const char* GroovesharkService::kSettingsGroup = "Grooveshark";
const char* GroovesharkService::kUrl = "http://api.grooveshark.com/ws/3.0/";
const char* GroovesharkService::kUrlCover = "http://beta.grooveshark.com/static/amazonart/l";
const char* GroovesharkService::kHomepage = "http://grooveshark.com/";

const int GroovesharkService::kSongSearchLimit = 100;
const int GroovesharkService::kSongSimpleSearchLimit = 10;
const int GroovesharkService::kAlbumSearchLimit = 10;

const int GroovesharkService::kSearchDelayMsec = 400;

typedef QPair<QString, QVariant> Param;

GroovesharkService::GroovesharkService(Application* app, InternetModel *parent)
  : InternetService(kServiceName, app, parent, parent),
    url_handler_(new GroovesharkUrlHandler(this, this)),
    next_pending_search_id_(0),
    root_(NULL),
    search_(NULL),
    popular_month_(NULL),
    popular_today_(NULL),
    stations_(NULL),
    grooveshark_radio_(NULL),
    favorites_(NULL),
    library_(NULL),
    playlists_parent_(NULL),
    subscribed_playlists_parent_(NULL),
    network_(new NetworkAccessManager(this)),
    context_menu_(NULL),
    create_playlist_(NULL),
    delete_playlist_(NULL),
    rename_playlist_(NULL),
    remove_from_playlist_(NULL),
    remove_from_favorites_(NULL),
    remove_from_library_(NULL),
    get_url_to_share_song_(NULL),
    get_url_to_share_playlist_(NULL),
    search_box_(new SearchBoxWidget(this)),
    search_delay_(new QTimer(this)),
    last_search_reply_(NULL),
    api_key_(QByteArray::fromBase64(kApiSecret)),
    login_state_(LoginState_OtherError),
    task_popular_id_(0),
    task_playlists_id_(0),
    task_search_id_(0) {

  app_->player()->RegisterUrlHandler(url_handler_);

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));

  // Get already existing (authenticated) session id, if any
  QSettings s;
  s.beginGroup(GroovesharkService::kSettingsGroup);
  session_id_ = s.value("sessionid").toString();
  username_ = s.value("username").toString();

  GroovesharkSearchProvider* search_provider = new GroovesharkSearchProvider(app_, this);
  search_provider->Init(this);
  app_->global_search()->AddProvider(search_provider);

  // Init secret: this code is ugly, but that's good as nobody is supposed to wonder what it does
  QByteArray ba = QByteArray::fromBase64(QCoreApplication::applicationName().toLatin1());
  int n = api_key_.length(), n2 = ba.length();
  for (int i=0; i<n; i++) api_key_[i] = api_key_[i] ^ ba[i%n2];

  connect(search_box_, SIGNAL(TextChanged(QString)), SLOT(Search(QString)));
}


GroovesharkService::~GroovesharkService() {
}

QStandardItem* GroovesharkService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/grooveshark.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  root_->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                           InternetModel::Role_PlayBehaviour);
  return root_;
}

void GroovesharkService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service: {
      EnsureConnected();
      break;
    }
    default:
      break;
  }
}

void GroovesharkService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Grooveshark);
}

QWidget* GroovesharkService::HeaderWidget() const {
  if (IsLoggedIn())
    return search_box_;
  return NULL;
}

void GroovesharkService::Search(const QString& text, bool now) {
  pending_search_ = text;

  // If there is no text (e.g. user cleared search box), we don't need to do a
  // real query that will return nothing: we can clear the playlist now
  if (text.isEmpty()) {
    search_delay_->stop();
    ClearSearchResults();
    return;
  }

  if (now) {
    search_delay_->stop();
    DoSearch();
  } else {
    search_delay_->start();
  }
}

int GroovesharkService::SimpleSearch(const QString& query) {
  QList<Param> parameters;
  parameters << Param("query", query)
             << Param("country", "")
             << Param("limit", QString::number(kSongSimpleSearchLimit))
             << Param("offset", "");

  int id = next_pending_search_id_++;
  QNetworkReply* reply = CreateRequest("getSongSearchResults", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SimpleSearchFinished(QNetworkReply*, int)), reply, id);
  return id;
}

void GroovesharkService::SimpleSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  emit SimpleSearchResults(id, songs);
}

int GroovesharkService::SearchAlbums(const QString& query) {
  QList<Param> parameters;
  parameters << Param("query", query)
             << Param("country", "")
             << Param("limit", QString::number(kAlbumSearchLimit));

  QNetworkReply* reply = CreateRequest("getAlbumSearchResults", parameters);

  const int id = next_pending_search_id_++;

  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(SearchAlbumsFinished(QNetworkReply*,int)),
             reply, id);

  return id;
}

void GroovesharkService::SearchAlbumsFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  QVariantList albums = result["albums"].toList();

  QList<quint64> ret;
  foreach (const QVariant& v, albums) {
    quint64 album_id = v.toMap()["AlbumID"].toULongLong();
    GetAlbumSongs(album_id);
    ret << album_id;
  }

  emit AlbumSearchResult(id, ret);
}

void GroovesharkService::GetAlbumSongs(quint64 album_id) {
  QList<Param> parameters;
  parameters << Param("albumID", album_id)
             << Param("country", "");
  QNetworkReply* reply = CreateRequest("getAlbumSongs", parameters);
  NewClosure(reply, SIGNAL(finished()),
    this, SLOT(GetAlbumSongsFinished(QNetworkReply*,quint64)),
    reply, album_id);
}

void GroovesharkService::GetAlbumSongsFinished(QNetworkReply* reply, quint64 album_id) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);

  emit AlbumSongsLoaded(album_id, songs);
}

void GroovesharkService::DoSearch() {
  if (!task_search_id_) {
    task_search_id_ = app_->task_manager()->StartTask(tr("Searching on Grooveshark"));
  }

  ClearSearchResults();

  QList<Param> parameters;
  parameters  << Param("query", pending_search_)
              << Param("country", "")
              << Param("limit", QString::number(kSongSearchLimit))
              << Param("offset", "");
  last_search_reply_ = CreateRequest("getSongSearchResults", parameters);
  NewClosure(last_search_reply_, SIGNAL(finished()), this,
             SLOT(SearchSongsFinished(QNetworkReply*)), last_search_reply_);
}

void GroovesharkService::SearchSongsFinished(QNetworkReply* reply) {
  reply->deleteLater();

  if (reply != last_search_reply_)
    return;

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  app_->task_manager()->SetTaskFinished(task_search_id_);
  task_search_id_ = 0;

  // Fill results list
  foreach (const Song& song, songs) {
    QStandardItem* child = CreateSongItem(song);
    search_->appendRow(child);
  }

  QModelIndex index = model()->merged_model()->mapFromSource(search_->index());
  ScrollToIndex(index);
}

void GroovesharkService::InitCountry() {
  if (!country_.isEmpty())
    return;
  // Get country info
  QNetworkReply *reply_country = CreateRequest("getCountry", QList<Param>());
  if (WaitForReply(reply_country)) {
    country_ = ExtractResult(reply_country);
  }
  reply_country->deleteLater();
}

QUrl GroovesharkService::GetStreamingUrlFromSongId(const QString& song_id, const QString& artist_id,
    QString* server_id, QString* stream_key, qint64* length_nanosec) {
  QList<Param> parameters;

  InitCountry();
  parameters  << Param("songID", song_id)
              << Param("country", country_);
  QNetworkReply* reply = CreateRequest("getSubscriberStreamKey", parameters);

  // Wait for the reply
  bool reply_has_timeouted = !WaitForReply(reply);
  reply->deleteLater();
  if (reply_has_timeouted)
    return QUrl();

  QVariantMap result = ExtractResult(reply);
  server_id->clear();
  server_id->append(result["StreamServerID"].toString());
  stream_key->clear();
  stream_key->append(result["StreamKey"].toString());
  *length_nanosec = result["uSecs"].toLongLong() * 1000;
  // Keep in mind that user has request to listen to this song
  last_songs_ids_.append(song_id.toInt());
  last_artists_ids_.append(artist_id.toInt());
  // If we have enough ids, remove the old ones
  if (last_songs_ids_.size() > 100)
    last_songs_ids_.removeFirst();
  if (last_artists_ids_.size() > 100)
    last_artists_ids_.removeFirst();

  return QUrl(result["url"].toString());
}

void GroovesharkService::Login(const QString& username, const QString& password) {
  // To login, we first need to create a session. Next, we will authenticate
  // this session using the user's username and password (for now, we just keep
  // them in mind)
  username_ = username;
  password_ = QCryptographicHash::hash(password.toLocal8Bit(), QCryptographicHash::Md5).toHex();

  QList<Param> parameters;
  QNetworkReply *reply = CreateRequest("startSession", parameters, true);

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SessionCreated(QNetworkReply*)), reply);
}

void GroovesharkService::SessionCreated(QNetworkReply* reply) {
  reply->deleteLater();

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
    emit StreamError("Failed to create Grooveshark session: " +
                     reply->errorString());
    emit LoginFinished(false);
    return;
  }

  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Error) << "Grooveshark returned an error during session creation";
  }
  session_id_ = result["sessionID"].toString();
  qLog(Debug) << "Session ID returned: " << session_id_;

  AuthenticateSession();
}

void GroovesharkService::AuthenticateSession() {
  QList<Param> parameters;
  parameters  << Param("login", username_)
              << Param("password", password_);

  QNetworkReply *reply = CreateRequest("authenticate", parameters, true);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(Authenticated(QNetworkReply*)), reply);
}

void GroovesharkService::Authenticated(QNetworkReply* reply) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  // Check if the user has been authenticated correctly
  QString error;
  if (!result["success"].toBool() || result["UserID"].toInt() == 0) {
    error = tr("Invalid username and/or password");
    login_state_ = LoginState_AuthFailed;
  } else if(!result["IsAnywhere"].toBool() || !result["IsPremium"].toBool()) {
    error = tr("User %1 doesn't have a Grooveshark Anywhere account").arg(username_);
    login_state_ = LoginState_NoPremium;
  }
  if (!error.isEmpty()) {
    QMessageBox::warning(NULL, tr("Grooveshark login error"), error, QMessageBox::Close);
    ResetSessionId();
    emit LoginFinished(false);
    return;
  }
  login_state_ = LoginState_LoggedIn;
  user_id_ = result["UserID"].toString();
  emit LoginFinished(true);
  EnsureItemsCreated();
}

void GroovesharkService::ClearSearchResults() {
  if (search_)
    search_->removeRows(0, search_->rowCount());
}

void GroovesharkService::Logout() {
  ResetSessionId();
  RemoveItems();
}

void GroovesharkService::RemoveItems() {
  root_->removeRows(0, root_->rowCount());
  // 'search', 'favorites', 'popular', ... items were root's children, and have
  // been deleted: we should update these now invalid pointers
  search_ = NULL;
  popular_month_ = NULL;
  popular_today_ = NULL;
  library_ = NULL;
  favorites_ = NULL;
  subscribed_playlists_parent_ = NULL;
  stations_ = NULL;
  grooveshark_radio_ = NULL;
  playlists_parent_ = NULL;
  playlists_.clear();
  subscribed_playlists_parent_ = NULL;
  subscribed_playlists_.clear();
  pending_retrieve_playlists_.clear();
}

void GroovesharkService::ResetSessionId() {
  QSettings s;
  s.beginGroup(GroovesharkService::kSettingsGroup);

  session_id_.clear();
  s.setValue("sessionid", session_id_);
}

void GroovesharkService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  // Check if we should display actions
  bool  display_delete_playlist_action = false,
        display_remove_from_playlist_action = false,
        display_remove_from_favorites_action = false,
        display_remove_from_library_action = false,
        display_share_song_url = false,
        display_share_playlist_url = false;

  QModelIndex index(model()->current_index());

  if (index.data(InternetModel::Role_Type).toInt() == InternetModel::Type_UserPlaylist &&
      index.data(Role_PlaylistType).toInt() == UserPlaylist) {
    display_delete_playlist_action = true;
  }
  // We check parent's type (instead of index type) because we want to enable
  // 'remove' actions for items which are inside a playlist
  int parent_type = index.parent().data(InternetModel::Role_Type).toInt();
  if (parent_type == InternetModel::Type_UserPlaylist) {
    int parent_playlist_type = index.parent().data(Role_PlaylistType).toInt();
    if (parent_playlist_type == UserFavorites)
      display_remove_from_favorites_action = true;
    else if (parent_playlist_type == UserLibrary)
      display_remove_from_library_action = true;
    else if (parent_playlist_type == UserPlaylist)
      display_remove_from_playlist_action = true;
  }
  delete_playlist_->setVisible(display_delete_playlist_action);
  // If we can delete this playlist, we can also rename it
  rename_playlist_->setVisible(display_delete_playlist_action);
  remove_from_playlist_->setVisible(display_remove_from_playlist_action);
  remove_from_favorites_->setVisible(display_remove_from_favorites_action);
  remove_from_library_->setVisible(display_remove_from_library_action);

  // Check if we can display actions to get URL for sharing songs/playlists:
  // - share song
  if (index.data(InternetModel::Role_Type).toInt() == InternetModel::Type_Track) {
    display_share_song_url = true;
    current_song_id_ = ExtractSongId(index.data(InternetModel::Role_Url).toUrl());
  }
  get_url_to_share_song_->setVisible(display_share_song_url);

  // - share playlist
  if (index.data(InternetModel::Role_Type).toInt() == InternetModel::Type_UserPlaylist
      && index.data(Role_UserPlaylistId).isValid()) {
    display_share_playlist_url = true;
    current_playlist_id_ = index.data(Role_UserPlaylistId).toInt();
  } else if (parent_type == InternetModel::Type_UserPlaylist
      && index.parent().data(Role_UserPlaylistId).isValid()) {
    display_share_playlist_url = true;
    current_playlist_id_ = index.parent().data(Role_UserPlaylistId).toInt();
  }
  get_url_to_share_playlist_->setVisible(display_share_playlist_url);

  context_menu_->popup(global_pos);
}

void GroovesharkService::EnsureMenuCreated() {
  if(!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    create_playlist_ = context_menu_->addAction(
        IconLoader::Load("list-add"), tr("Create a new Grooveshark playlist"),
        this, SLOT(CreateNewPlaylist()));
    delete_playlist_ = context_menu_->addAction(
        IconLoader::Load("edit-delete"), tr("Delete Grooveshark playlist"),
        this, SLOT(DeleteCurrentPlaylist()));
    rename_playlist_ = context_menu_->addAction(
        IconLoader::Load("edit-rename"), tr("Rename Grooveshark playlist"),
        this, SLOT(RenameCurrentPlaylist()));
    context_menu_->addSeparator();
    remove_from_playlist_ = context_menu_->addAction(
        IconLoader::Load("list-remove"), tr("Remove from playlist"),
        this, SLOT(RemoveCurrentFromPlaylist()));
    remove_from_favorites_ = context_menu_->addAction(
        IconLoader::Load("list-remove"), tr("Remove from favorites"),
        this, SLOT(RemoveCurrentFromFavorites()));
    remove_from_library_ = context_menu_->addAction(
        IconLoader::Load("list-remove"), tr("Remove from My Music"),
        this, SLOT(RemoveCurrentFromLibrary()));
    get_url_to_share_song_ = context_menu_->addAction(
        tr("Get a URL to share this Grooveshark song"),
        this, SLOT(GetCurrentSongUrlToShare()));
    get_url_to_share_playlist_ = context_menu_->addAction(
        tr("Get a URL to share this Grooveshark playlist"),
        this, SLOT(GetCurrentPlaylistUrlToShare()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download"),
                             tr("Open %1 in browser").arg("grooveshark.com"),
                             this, SLOT(Homepage()));
    context_menu_->addAction(IconLoader::Load("view-refresh"),
                             tr("Refresh"), this, SLOT(RefreshItems()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure"),
                             tr("Configure Grooveshark..."),
                             this, SLOT(ShowConfig()));
  }
}

void GroovesharkService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

void GroovesharkService::RefreshItems() {
  RemoveItems();
  EnsureItemsCreated();
}

void GroovesharkService::EnsureItemsCreated() {
  if (IsLoggedIn() && !search_) {
    search_ = new QStandardItem(IconLoader::Load("edit-find"),
                                tr("Search results"));
    search_->setToolTip(tr("Start typing something on the search box above to "
                           "fill this search results list"));
    search_->setData(InternetModel::PlayBehaviour_MultipleItems,
                     InternetModel::Role_PlayBehaviour);
    root_->appendRow(search_);

    QStandardItem* popular = new QStandardItem(QIcon(":/star-on.png"),
                                               tr("Popular songs"));
    root_->appendRow(popular);

    popular_month_ = new QStandardItem(QIcon(":/star-on.png"), tr("Popular songs of the Month"));
    popular_month_->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
    popular_month_->setData(true, InternetModel::Role_CanLazyLoad);
    popular_month_->setData(InternetModel::PlayBehaviour_MultipleItems,
                        InternetModel::Role_PlayBehaviour);
    popular->appendRow(popular_month_);

    popular_today_ = new QStandardItem(QIcon(":/star-on.png"), tr("Popular songs today"));
    popular_today_->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
    popular_today_->setData(true, InternetModel::Role_CanLazyLoad);
    popular_today_->setData(InternetModel::PlayBehaviour_MultipleItems,
                        InternetModel::Role_PlayBehaviour);
    popular->appendRow(popular_today_);

    QStandardItem* radios_divider = new QStandardItem(QIcon(":last.fm/icon_radio.png"),
                                                      tr("Radios"));
    root_->appendRow(radios_divider);

    stations_ = new QStandardItem(QIcon(":last.fm/icon_radio.png"), tr("Stations"));
    stations_->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
    stations_->setData(true, InternetModel::Role_CanLazyLoad);
    radios_divider->appendRow(stations_);

    grooveshark_radio_ = new QStandardItem(QIcon(":last.fm/icon_radio.png"), tr("Grooveshark radio"));
    grooveshark_radio_->setToolTip(tr("Listen to Grooveshark songs based on what you've listened to previously"));
    grooveshark_radio_->setData(InternetModel::Type_SmartPlaylist, InternetModel::Role_Type);
    radios_divider->appendRow(grooveshark_radio_);

    library_ = new QStandardItem(IconLoader::Load("folder-sound"), tr("My Music"));
    library_->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
    library_->setData(UserLibrary, Role_PlaylistType);
    library_->setData(true, InternetModel::Role_CanLazyLoad);
    library_->setData(true, InternetModel::Role_CanBeModified);
    library_->setData(InternetModel::PlayBehaviour_MultipleItems,
                      InternetModel::Role_PlayBehaviour);
    root_->appendRow(library_);

    favorites_ = new QStandardItem(QIcon(":/last.fm/love.png"), tr("Favorites"));
    favorites_->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
    favorites_->setData(UserFavorites, Role_PlaylistType);
    favorites_->setData(true, InternetModel::Role_CanLazyLoad);
    favorites_->setData(true, InternetModel::Role_CanBeModified);
    favorites_->setData(InternetModel::PlayBehaviour_MultipleItems,
                        InternetModel::Role_PlayBehaviour);
    root_->appendRow(favorites_);

    playlists_parent_ = new QStandardItem(tr("Playlists"));
    root_->appendRow(playlists_parent_);

    subscribed_playlists_parent_ = new QStandardItem(tr("Subscribed playlists"));
    root_->appendRow(subscribed_playlists_parent_);

    RetrieveUserFavorites();
    RetrieveUserLibrarySongs();
    RetrieveUserPlaylists();
    RetrieveSubscribedPlaylists();
    RetrieveAutoplayTags();
    RetrievePopularSongs();
  }
}

void GroovesharkService::EnsureConnected() {
  if (session_id_.isEmpty()) {
    ShowConfig();
  } else {
    EnsureItemsCreated();
  }
}

QStandardItem* GroovesharkService::CreatePlaylistItem(const QString& playlist_name,
                                                      int playlist_id) {
  QStandardItem* item = new QStandardItem(playlist_name);
  item->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
  item->setData(UserPlaylist, Role_PlaylistType);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(true, InternetModel::Role_CanBeModified);
  item->setData(InternetModel::PlayBehaviour_MultipleItems, InternetModel::Role_PlayBehaviour);
  item->setData(playlist_id, Role_UserPlaylistId);
  return item;
}

void GroovesharkService::RetrieveUserPlaylists() {
  task_playlists_id_ =
    app_->task_manager()->StartTask(tr("Retrieving Grooveshark playlists"));
  QNetworkReply* reply = CreateRequest("getUserPlaylists", QList<Param>());

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(UserPlaylistsRetrieved(QNetworkReply*)), reply);
}

void GroovesharkService::UserPlaylistsRetrieved(QNetworkReply* reply) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  QList<PlaylistInfo> playlists = ExtractPlaylistInfo(result);

  foreach(const PlaylistInfo& playlist_info, playlists) {
    int playlist_id = playlist_info.id_;
    const QString& playlist_name = playlist_info.name_;
    QStandardItem* playlist_item =
        CreatePlaylistItem(playlist_name, playlist_id);
    playlists_parent_->appendRow(playlist_item);

    // Keep in mind this playlist
    playlists_.insert(playlist_id,
                      PlaylistInfo(playlist_id, playlist_name, playlist_item));

    // Request playlist's songs
    RefreshPlaylist(playlist_id);
  }

  if (playlists.isEmpty()) {
    app_->task_manager()->SetTaskFinished(task_playlists_id_);
  }
}

void GroovesharkService::PlaylistSongsRetrieved(
    QNetworkReply* reply, int playlist_id) {
  reply->deleteLater();
  pending_retrieve_playlists_.remove(playlist_id);
  PlaylistInfo* playlist_info = subscribed_playlists_.contains(playlist_id) ?
      &subscribed_playlists_[playlist_id] : &playlists_[playlist_id];
  playlist_info->item_->removeRows(0, playlist_info->item_->rowCount());

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  Song::SortSongsListAlphabetically(&songs);

  foreach (const Song& song, songs) {
    QStandardItem* child = CreateSongItem(song);
    child->setData(playlist_info->id_, Role_UserPlaylistId);
    child->setData(true, InternetModel::Role_CanBeModified);

    playlist_info->item_->appendRow(child);
  }

  // Keep in mind this playlist
  playlist_info->songs_ids_ = ExtractSongsIds(result);

  if (pending_retrieve_playlists_.isEmpty()) {
    app_->task_manager()->SetTaskFinished(task_playlists_id_);
  }
}

void GroovesharkService::RetrieveUserFavorites() {
  int task_id =
    app_->task_manager()->StartTask(tr("Retrieving Grooveshark favorites songs"));
  QNetworkReply* reply = CreateRequest("getUserFavoriteSongs", QList<Param>());

  NewClosure(reply, SIGNAL(finished()),
      this, SLOT(UserFavoritesRetrieved(QNetworkReply*, int)), reply, task_id);
}

void GroovesharkService::UserFavoritesRetrieved(QNetworkReply* reply, int task_id) {
  reply->deleteLater();
  app_->task_manager()->SetTaskFinished(task_id);

  if (!favorites_) {
    // The use probably logged out before the response arrived.
    return;
  }

  favorites_->removeRows(0, favorites_->rowCount());

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  Song::SortSongsListAlphabetically(&songs);

  foreach (const Song& song, songs) {
    QStandardItem* child = CreateSongItem(song);
    child->setData(true, InternetModel::Role_CanBeModified);

    favorites_->appendRow(child);
  }
}

void GroovesharkService::RetrieveUserLibrarySongs() {
  int task_id =
    app_->task_manager()->StartTask(tr("Retrieving Grooveshark My Music songs"));
  QNetworkReply* reply = CreateRequest("getUserLibrarySongs", QList<Param>());

  NewClosure(reply, SIGNAL(finished()),
      this, SLOT(UserLibrarySongsRetrieved(QNetworkReply*, int)), reply, task_id);
}

void GroovesharkService::UserLibrarySongsRetrieved(QNetworkReply* reply, int task_id) {
  reply->deleteLater();
  app_->task_manager()->SetTaskFinished(task_id);

  if (!library_) {
    // The use probably logged out before the response arrived.
    return;
  }

  library_->removeRows(0, library_->rowCount());

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  Song::SortSongsListAlphabetically(&songs);

  foreach (const Song& song, songs) {
    QStandardItem* child = CreateSongItem(song);
    child->setData(true, InternetModel::Role_CanBeModified);

    library_->appendRow(child);
  }
}

void GroovesharkService::RetrievePopularSongs() {
  task_popular_id_ =
    app_->task_manager()->StartTask(tr("Getting Grooveshark popular songs"));
  RetrievePopularSongsMonth();
  RetrievePopularSongsToday();
}

void GroovesharkService::RetrievePopularSongsMonth() {
  QList<Param> parameters;
  parameters  << Param("limit", QString::number(kSongSearchLimit));
  QNetworkReply* reply = CreateRequest("getPopularSongsMonth", parameters);
  NewClosure(reply, SIGNAL(finished()),
      this, SLOT(PopularSongsMonthRetrieved(QNetworkReply*)), reply);
}

void GroovesharkService::PopularSongsMonthRetrieved(QNetworkReply* reply) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);

  app_->task_manager()->IncreaseTaskProgress(task_popular_id_, 50, 100);
  if (app_->task_manager()->GetTaskProgress(task_popular_id_) >= 100) {
    app_->task_manager()->SetTaskFinished(task_popular_id_);
  }

  if (!popular_month_)
    return;

  foreach (const Song& song, songs) {
    QStandardItem* child = CreateSongItem(song);
    popular_month_->appendRow(child);
  }
}

void GroovesharkService::RetrievePopularSongsToday() {
  QList<Param> parameters;
  parameters  << Param("limit", QString::number(kSongSearchLimit));
  QNetworkReply* reply = CreateRequest("getPopularSongsToday", parameters);
  NewClosure(reply, SIGNAL(finished()),
      this, SLOT(PopularSongsTodayRetrieved(QNetworkReply*)), reply);
}

void GroovesharkService::PopularSongsTodayRetrieved(QNetworkReply* reply) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);

  app_->task_manager()->IncreaseTaskProgress(task_popular_id_, 50, 100);
  if (app_->task_manager()->GetTaskProgress(task_popular_id_) >= 100) {
    app_->task_manager()->SetTaskFinished(task_popular_id_);
  }

  if (!popular_today_)
    return;

  foreach (const Song& song, songs) {
    QStandardItem* child = CreateSongItem(song);
    popular_today_->appendRow(child);
  }
}

void GroovesharkService::RetrieveSubscribedPlaylists() {
  QNetworkReply* reply = CreateRequest("getUserPlaylistsSubscribed", QList<Param>());
  NewClosure(reply, SIGNAL(finished()),
      this, SLOT(SubscribedPlaylistsRetrieved(QNetworkReply*)), reply);
}

void GroovesharkService::SubscribedPlaylistsRetrieved(QNetworkReply* reply) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  QList<PlaylistInfo> playlists = ExtractPlaylistInfo(result);

  foreach(const PlaylistInfo& playlist_info, playlists) {
    int playlist_id = playlist_info.id_;
    const QString& playlist_name = playlist_info.name_;

    QStandardItem* playlist_item = CreatePlaylistItem(playlist_name, playlist_id);
    // Refine some playlist properties that should be different for subscribed
    // playlists
    playlist_item->setData(SubscribedPlaylist, Role_PlaylistType);
    playlist_item->setData(false, InternetModel::Role_CanBeModified);
    
    subscribed_playlists_.insert(playlist_id,
                                 PlaylistInfo(playlist_id, playlist_name, playlist_item));
    subscribed_playlists_parent_->appendRow(playlist_item);

    // Request playlist's songs
    RefreshPlaylist(playlist_id);
  }
}

void GroovesharkService::RetrieveAutoplayTags() {
  QNetworkReply* reply = CreateRequest("getAutoplayTags", QList<Param>());
  NewClosure(reply, SIGNAL(finished()),
      this, SLOT(AutoplayTagsRetrieved(QNetworkReply*)), reply);
}

void GroovesharkService::AutoplayTagsRetrieved(QNetworkReply* reply) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  QVariantMap::const_iterator it;
  if (!stations_)
    return;
  for (it = result.constBegin(); it != result.constEnd(); ++it) {
    int id = it.key().toInt();
    QString name = it.value().toString().toLower();
    // Names received aren't very nice: make them more user friendly to display
    name.replace("_", " ");
    name[0] = name[0].toUpper();

    QStandardItem* item = new QStandardItem(QIcon(":last.fm/icon_radio.png"), name);
    item->setData(InternetModel::Type_SmartPlaylist, InternetModel::Role_Type);
    item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
    item->setData(id, Role_UserPlaylistId);

    stations_->appendRow(item);
  }
}

Song GroovesharkService::StartAutoplayTag(int tag_id, QVariantMap& autoplay_state) {
  QList<Param> parameters;
  parameters << Param("tagID", tag_id);
  QNetworkReply* reply = CreateRequest("startAutoplayTag", parameters);

  bool reply_has_timeouted = !WaitForReply(reply);
  reply->deleteLater();
  if (reply_has_timeouted)
    return Song();

  QVariantMap result = ExtractResult(reply);
  autoplay_state = result["autoplayState"].toMap();
  return ExtractSong(result["nextSong"].toMap());
}

Song GroovesharkService::StartAutoplay(QVariantMap& autoplay_state) {
  QList<Param> parameters;
  QVariantList artists_ids_qvariant;
  foreach (int artist_id, last_artists_ids_) {
    artists_ids_qvariant << QVariant(artist_id);
  }
  QVariantList songs_ids_qvariant;
  foreach (int song_id, last_songs_ids_) {
    songs_ids_qvariant << QVariant(song_id);
  }
  parameters  << Param("artistIDs", artists_ids_qvariant)
              << Param("songIDs", songs_ids_qvariant);
  QNetworkReply* reply = CreateRequest("startAutoplay", parameters);

  bool reply_has_timeouted = !WaitForReply(reply);
  reply->deleteLater();
  if (reply_has_timeouted)
    return Song();

  QVariantMap result = ExtractResult(reply);
  autoplay_state = result["autoplayState"].toMap();
  return ExtractSong(result["nextSong"].toMap());
}

Song GroovesharkService::GetAutoplaySong(QVariantMap& autoplay_state) {
  QList<Param> parameters;
  parameters << Param("autoplayState", autoplay_state);
  QNetworkReply* reply = CreateRequest("getAutoplaySong", parameters);

  bool reply_has_timeouted = !WaitForReply(reply);
  reply->deleteLater();
  if (reply_has_timeouted)
    return Song();

  QVariantMap result = ExtractResult(reply);
  autoplay_state = result["autoplayState"].toMap();
  return ExtractSong(result["nextSong"].toMap());
}

void GroovesharkService::MarkStreamKeyOver30Secs(const QString& stream_key,
                                                 const QString& server_id) {
  QList<Param> parameters;
  parameters  << Param("streamKey", stream_key)
              << Param("streamServerID", server_id);

  QNetworkReply* reply = CreateRequest("markStreamKeyOver30Secs", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(StreamMarked(QNetworkReply*)), reply);
}

void GroovesharkService::StreamMarked(QNetworkReply* reply) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark markStreamKeyOver30Secs failed";
  }
}

void GroovesharkService::MarkSongComplete(const QString& song_id,
                                          const QString& stream_key,
                                          const QString& server_id) {
  QList<Param> parameters;
  parameters  << Param("songID", song_id)
              << Param("streamKey", stream_key)
              << Param("streamServerID", server_id);

  QNetworkReply* reply = CreateRequest("markSongComplete", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SongMarkedAsComplete(QNetworkReply*)), reply);
}

void GroovesharkService::SongMarkedAsComplete(QNetworkReply* reply) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark markSongComplete failed";
  }
}

void GroovesharkService::ItemDoubleClicked(QStandardItem* item) {
  if (item == root_) {
    EnsureConnected();
  }
}

GeneratorPtr GroovesharkService::CreateGenerator(QStandardItem* item) {
  GeneratorPtr ret;
  if (!item ||
      item->data(InternetModel::Role_Type).toInt() != InternetModel::Type_SmartPlaylist) {
    return ret;
  }

  if (item == grooveshark_radio_) {
    if (last_artists_ids_.isEmpty()) {
      QMessageBox::warning(NULL, tr("Error"),
        tr("To start Grooveshark radio, you should first listen to a few other Grooveshark songs"));
      return ret;
    }
    ret = GeneratorPtr(new GroovesharkRadio(this));
  } else {
    int tag_id = item->data(Role_UserPlaylistId).toInt();
    ret = GeneratorPtr(new GroovesharkRadio(this ,tag_id));
  }
  return ret;
}

void GroovesharkService::DropMimeData(const QMimeData* data, const QModelIndex& index) {
  if (!data) {
    return;
  }

  // Get Grooveshark songs' ids, if any.
  QList<int> data_songs_ids = ExtractSongsIds(data->urls());
  if (data_songs_ids.isEmpty()) {
    // There is none: probably means user didn't dropped Grooveshark songs
    return;
  }

  int type = index.data(InternetModel::Role_Type).toInt();
  int parent_type = index.parent().data(InternetModel::Role_Type).toInt();

  if (type == InternetModel::Type_UserPlaylist ||
      parent_type == InternetModel::Type_UserPlaylist) {
    int playlist_type = index.data(Role_PlaylistType).toInt();
    int parent_playlist_type = index.parent().data(Role_PlaylistType).toInt();
    // If dropped on Favorites list
    if (playlist_type == UserFavorites || parent_playlist_type == UserFavorites) {
      foreach (int song_id, data_songs_ids) {
        AddUserFavoriteSong(song_id);
      }
    } else if (playlist_type == UserLibrary || parent_playlist_type == UserLibrary) {
      // FIXME: Adding songs to user libray doesn't work atm, but the problem
      // seems to be on Grooveshark server side, as it returns success=true
      // when calling addUserLibrarySongs with a valid song id.
      // So this code is deactivated for now to not mislead user
      //AddUserLibrarySongs(data_songs_ids);
    } else { // Dropped on a normal playlist
      // Get the playlist
      int playlist_id = index.data(Role_UserPlaylistId).toInt();
      if (!playlists_.contains(playlist_id)) {
        return;
      }
      // Get the current playlist's songs
      PlaylistInfo playlist = playlists_[playlist_id];
      QList<int> songs_ids = playlist.songs_ids_;
      songs_ids << data_songs_ids;

      SetPlaylistSongs(playlist_id, songs_ids);
    }
  }
}

QList<QAction*> GroovesharkService::playlistitem_actions(const Song& song) {
  // Clear previous actions
  while (!playlistitem_actions_.isEmpty()) {
    QAction* action = playlistitem_actions_.takeFirst();
    QMenu* menu = action->menu();
    if (menu)
      delete menu;
    delete action;
  }

  // Create a 'add to favorites' action
  QAction* add_to_favorites = new QAction(QIcon(":/last.fm/love.png"),
                                          tr("Add to Grooveshark favorites"), this);
  connect(add_to_favorites, SIGNAL(triggered()), SLOT(AddCurrentSongToUserFavorites()));
  playlistitem_actions_.append(add_to_favorites);

  // FIXME: as explained above, adding songs to library doesn't work currently
  //QAction* add_to_library = new QAction(IconLoader::Load("folder-sound"),
  //                                      tr("Add to Grooveshark My Music"), this);
  //connect(add_to_library, SIGNAL(triggered()), SLOT(AddCurrentSongToUserLibrary()));
  //playlistitem_actions_.append(add_to_library);

  // Create a menu with 'add to playlist' actions for each Grooveshark playlist
  QAction* add_to_playlists = new QAction(IconLoader::Load("list-add"),
                                          tr("Add to Grooveshark playlists"), this);
  QMenu* playlists_menu = new QMenu();
  foreach (PlaylistInfo playlist_info, playlists_.values()) {
    QAction* add_to_playlist = new QAction(playlist_info.name_, this);
    add_to_playlist->setData(playlist_info.id_);
    playlists_menu->addAction(add_to_playlist);
  }
  connect(playlists_menu, SIGNAL(triggered(QAction*)), SLOT(AddCurrentSongToPlaylist(QAction*)));
  add_to_playlists->setMenu(playlists_menu);
  playlistitem_actions_.append(add_to_playlists);

  QAction* share_song = new QAction(tr("Get a URL to share this Grooveshark song"), this);
  connect(share_song, SIGNAL(triggered()), SLOT(GetCurrentSongUrlToShare()));
  playlistitem_actions_.append(share_song);

  // Keep in mind the current song id
  current_song_id_ = ExtractSongId(song.url());

  return playlistitem_actions_;
}

void GroovesharkService::GetCurrentSongUrlToShare() {
  GetSongUrlToShare(current_song_id_);
}

void GroovesharkService::GetSongUrlToShare(int song_id) {
  QList<Param> parameters;
  parameters << Param("songID", song_id);
  QNetworkReply* reply = CreateRequest("getSongURLFromSongID", parameters);

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SongUrlToShareReceived(QNetworkReply*)), reply);
}

void GroovesharkService::SongUrlToShareReceived(QNetworkReply* reply) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  if (!result["url"].isValid())
    return;
  QString url = result["url"].toString();
  ShowUrlBox(tr("Grooveshark song's URL"), url);
}

void GroovesharkService::GetCurrentPlaylistUrlToShare() {
  GetPlaylistUrlToShare(current_playlist_id_);
}

void GroovesharkService::GetPlaylistUrlToShare(int playlist_id) {
  QList<Param> parameters;
  parameters << Param("playlistID", playlist_id);
  QNetworkReply* reply = CreateRequest("getPlaylistURLFromPlaylistID", parameters);

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(PlaylistUrlToShareReceived(QNetworkReply*)), reply);
}

void GroovesharkService::PlaylistUrlToShareReceived(QNetworkReply* reply) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["url"].isValid())
    return;
  QString url = result["url"].toString();
  ShowUrlBox(tr("Grooveshark playlist's URL"), url);
}

void GroovesharkService::ShowUrlBox(const QString& title, const QString& url) {
  QMessageBox url_box;
  url_box.setWindowTitle(title);
  url_box.setWindowIcon(QIcon(":/icon.png"));
  url_box.setText(url);
  url_box.setStandardButtons(QMessageBox::Ok);
  QPushButton* copy_to_clipboard_button =
      url_box.addButton(tr("Copy to clipboard"), QMessageBox::ActionRole);

  url_box.exec();

  if (url_box.clickedButton() == copy_to_clipboard_button) {
    QApplication::clipboard()->setText(url);
  }
}

void GroovesharkService::AddCurrentSongToPlaylist(QAction* action) {
  int playlist_id = action->data().toInt();
  if (!playlists_.contains(playlist_id)) {
    return;
  }
  // Get the current playlist's songs
  PlaylistInfo playlist = playlists_[playlist_id];
  QList<int> songs_ids = playlist.songs_ids_;
  songs_ids << current_song_id_;

  SetPlaylistSongs(playlist_id, songs_ids);
}

void GroovesharkService::SetPlaylistSongs(int playlist_id, const QList<int>& songs_ids) {
  // If we are still retrieving playlists songs, don't update playlist: don't
  // take the risk to erase all (not yet retrieved) playlist's songs.
  if (!pending_retrieve_playlists_.isEmpty())
    return;
  int task_id =
    app_->task_manager()->StartTask(tr("Update Grooveshark playlist"));

  QList<Param> parameters;

  // Convert song ids to QVariant
  QVariantList songs_ids_qvariant;
  foreach (int song_id, songs_ids) {
    songs_ids_qvariant << QVariant(song_id);
  }

  parameters  << Param("playlistID", playlist_id)
              << Param("songIDs", songs_ids_qvariant);

  QNetworkReply* reply = CreateRequest("setPlaylistSongs", parameters);

  NewClosure(reply, SIGNAL(finished()),
    this, SLOT(PlaylistSongsSet(QNetworkReply*, int, int)),
    reply, playlist_id, task_id);
}

void GroovesharkService::PlaylistSongsSet(QNetworkReply* reply, int playlist_id, int task_id) {
  reply->deleteLater();
  app_->task_manager()->SetTaskFinished(task_id);

  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark setPlaylistSongs failed";
    return;
  }

  RefreshPlaylist(playlist_id);
}

void GroovesharkService::RefreshPlaylist(int playlist_id) {
  QList<Param> parameters;
  parameters << Param("playlistID", playlist_id);
  QNetworkReply* reply = CreateRequest("getPlaylistSongs", parameters); 
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(PlaylistSongsRetrieved(QNetworkReply*, int)), reply, playlist_id);
  pending_retrieve_playlists_.insert(playlist_id);
}

void GroovesharkService::CreateNewPlaylist() {
  QString name = QInputDialog::getText(NULL,
                                       tr("Create a new Grooveshark playlist"),
                                       tr("Name"),
                                       QLineEdit::Normal);
  if (name.isEmpty()) {
    return;
  }

  QList<Param> parameters;
  parameters << Param("name", name)
             << Param("songIDs", QVariantList());
  QNetworkReply* reply = CreateRequest("createPlaylist", parameters);
  NewClosure(reply, SIGNAL(finished()),
    this, SLOT(NewPlaylistCreated(QNetworkReply*, const QString&)), reply, name);
}

void GroovesharkService::NewPlaylistCreated(QNetworkReply* reply, const QString& name) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool() || !result["playlistID"].isValid()) {
    qLog(Warning) << "Grooveshark createPlaylist failed";
    return;
  }

  int playlist_id = result["playlistID"].toInt();
  QStandardItem* new_playlist_item = CreatePlaylistItem(name, playlist_id);
  PlaylistInfo playlist_info(playlist_id, name, new_playlist_item);
  playlist_info.item_ = new_playlist_item;
  playlists_parent_->appendRow(new_playlist_item);
  playlists_.insert(playlist_id, playlist_info);
}

void GroovesharkService::DeleteCurrentPlaylist() {
  if (model()->current_index().data(InternetModel::Role_Type).toInt() !=
      InternetModel::Type_UserPlaylist) {
    return;
  }

  int playlist_id = model()->current_index().data(Role_UserPlaylistId).toInt();
  DeletePlaylist(playlist_id);
}

void GroovesharkService::DeletePlaylist(int playlist_id) {
  if (!playlists_.contains(playlist_id)) {
    return;
  }
  
  boost::scoped_ptr<QMessageBox> confirmation_dialog(new QMessageBox(
      QMessageBox::Question, tr("Delete Grooveshark playlist"),
      tr("Are you sure you want to delete this playlist?"),
      QMessageBox::Yes | QMessageBox::Cancel));
  if (confirmation_dialog->exec() != QMessageBox::Yes) {
    return;
  }

  QList<Param> parameters;
  parameters << Param("playlistID", playlist_id);
  QNetworkReply* reply = CreateRequest("deletePlaylist", parameters);
  NewClosure(reply, SIGNAL(finished()),
    this, SLOT(PlaylistDeleted(QNetworkReply*, int)), reply, playlist_id);
}

void GroovesharkService::PlaylistDeleted(QNetworkReply* reply, int playlist_id) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark deletePlaylist failed";
    return;
  }
  if (!playlists_.contains(playlist_id)) {
    return;
  }
  PlaylistInfo playlist_info = playlists_.take(playlist_id);
  playlists_parent_->removeRow(playlist_info.item_->row());
}

void GroovesharkService::RenameCurrentPlaylist() {
  const QModelIndex& index(model()->current_index());

  if (index.data(InternetModel::Role_Type).toInt() != InternetModel::Type_UserPlaylist
      || index.data(Role_PlaylistType).toInt() != UserPlaylist) {
    return;
  }

  const int playlist_id = index.data(Role_UserPlaylistId).toInt();
  RenamePlaylist(playlist_id);
}

void GroovesharkService::RenamePlaylist(int playlist_id) {
  if (!playlists_.contains(playlist_id)) {
    return;
  }
  const QString& old_name = playlists_[playlist_id].name_;
  QString new_name = QInputDialog::getText(NULL,
                                       tr("Rename \"%1\" playlist").arg(old_name),
                                       tr("Name"),
                                       QLineEdit::Normal,
                                       old_name);
  if (new_name.isEmpty()) {
    return;
  }

  QList<Param> parameters;
  parameters  << Param("playlistID", playlist_id)
              << Param("name", new_name);
  QNetworkReply* reply = CreateRequest("renamePlaylist", parameters);
  NewClosure(reply, SIGNAL(finished()),
    this, SLOT(PlaylistRenamed(QNetworkReply*, int, const QString&)), reply, playlist_id, new_name);
}

void GroovesharkService::PlaylistRenamed(QNetworkReply* reply,
                                         int playlist_id,
                                         const QString& new_name) {
  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark renamePlaylist failed";
    return;
  }
  if (!playlists_.contains(playlist_id)) {
    return;
  }
  PlaylistInfo& playlist_info = playlists_[playlist_id];
  playlist_info.name_ = new_name;
  playlist_info.item_->setText(new_name);
}

void GroovesharkService::AddUserFavoriteSong(int song_id) {
  int task_id = app_->task_manager()->StartTask(tr("Adding song to favorites"));
  QList<Param> parameters;
  parameters << Param("songID", song_id);
  QNetworkReply* reply = CreateRequest("addUserFavoriteSong", parameters);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(UserFavoriteSongAdded(QNetworkReply*, int)),
             reply, task_id);
}

void GroovesharkService::UserFavoriteSongAdded(QNetworkReply* reply, int task_id) {
  reply->deleteLater();
  app_->task_manager()->SetTaskFinished(task_id);

  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark addUserFavoriteSong failed";
    return;
  }
  // Refresh user's favorites list
  RetrieveUserFavorites();
}

void GroovesharkService::AddUserLibrarySongs(const QList<int>& songs_ids) {
  int task_id = app_->task_manager()->StartTask(tr("Adding song to My Music"));
  QList<Param> parameters;

  // Convert songs ids to QVariant
  QVariantList songs_ids_qvariant;
  foreach (int song_id, songs_ids) {
    songs_ids_qvariant << QVariant(song_id);
  }
  QVariantList albums_ids_qvariant;
  QVariantList artists_ids_qvariant;

  parameters << Param("songIDs", songs_ids_qvariant);
  // We do not support albums and artist parameters for now, but they are
  // required
  parameters << Param("albumIDs", albums_ids_qvariant);
  parameters << Param("artistIDs", artists_ids_qvariant);
  QNetworkReply* reply = CreateRequest("addUserLibrarySongs", parameters);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(UserLibrarySongAdded(QNetworkReply*, int)),
             reply, task_id);
}

void GroovesharkService::UserLibrarySongAdded(QNetworkReply* reply, int task_id) {
  reply->deleteLater();
  app_->task_manager()->SetTaskFinished(task_id);

  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark addUserLibrarySongs failed";
    return;
  }
  // Refresh user's library list
  RetrieveUserLibrarySongs();
}

void GroovesharkService::RemoveCurrentFromPlaylist() {
  const QModelIndexList& indexes(model()->selected_indexes());
  QMap<int, QList<int> > playlists_songs_ids;
  foreach (const QModelIndex& index, indexes) {

    if (index.parent().data(InternetModel::Role_Type).toInt() !=
        InternetModel::Type_UserPlaylist) {
      continue;
    }

    int playlist_id = index.data(Role_UserPlaylistId).toInt();
    int song_id = ExtractSongId(index.data(InternetModel::Role_Url).toUrl());
    if (song_id) {
      playlists_songs_ids[playlist_id] << song_id;
    }
  }

  for (QMap<int, QList<int> >::const_iterator it = playlists_songs_ids.constBegin();
       it != playlists_songs_ids.constEnd();
       ++it) {
    RemoveFromPlaylist(it.key(), it.value());
  }
}

void GroovesharkService::RemoveFromPlaylist(int playlist_id,
                                            const QList<int>& songs_ids_to_remove) {
  if (!playlists_.contains(playlist_id)) {
    return;
  }

  QList<int> songs_ids = playlists_[playlist_id].songs_ids_;
  foreach (const int song_id, songs_ids_to_remove) {
    songs_ids.removeOne(song_id);
  }

  SetPlaylistSongs(playlist_id, songs_ids);
}

void GroovesharkService::RemoveCurrentFromFavorites() {
  const QModelIndexList& indexes(model()->selected_indexes());
  QList<int> songs_ids;
  foreach (const QModelIndex& index, indexes) {

    if (index.parent().data(Role_PlaylistType).toInt() != UserFavorites) {
      continue;
    }

    int song_id = ExtractSongId(index.data(InternetModel::Role_Url).toUrl());
    if (song_id) {
      songs_ids << song_id;
    }
  }
  
  RemoveFromFavorites(songs_ids);
}

void GroovesharkService::RemoveFromFavorites(const QList<int>& songs_ids_to_remove) {
  if (songs_ids_to_remove.isEmpty())
    return;

  int task_id = app_->task_manager()->StartTask(tr("Removing songs from favorites"));
  QList<Param> parameters;

  // Convert song ids to QVariant
  QVariantList songs_ids_qvariant;
  foreach (const int song_id, songs_ids_to_remove) {
    songs_ids_qvariant << QVariant(song_id);
  }

  parameters << Param("songIDs", songs_ids_qvariant);
  QNetworkReply* reply = CreateRequest("removeUserFavoriteSongs", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
    SLOT(SongsRemovedFromFavorites(QNetworkReply*, int)), reply, task_id);
}

void GroovesharkService::SongsRemovedFromFavorites(QNetworkReply* reply, int task_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark removeUserFavoriteSongs failed";
    return;
  }
  RetrieveUserFavorites();
}

void GroovesharkService::RemoveCurrentFromLibrary() {
  const QModelIndexList& indexes(model()->selected_indexes());
  QList<int> songs_ids;

  foreach (const QModelIndex& index, indexes) {

    if (index.parent().data(Role_PlaylistType).toInt() != UserLibrary) {
      continue;
    }

    int song_id = ExtractSongId(index.data(InternetModel::Role_Url).toUrl());
    if (song_id) {
      songs_ids << song_id;
    }
  }
  
  RemoveFromLibrary(songs_ids);
}

void GroovesharkService::RemoveFromLibrary(const QList<int>& songs_ids_to_remove) {
  if (songs_ids_to_remove.isEmpty())
    return;

  int task_id = app_->task_manager()->StartTask(tr("Removing songs from My Music"));
  QList<Param> parameters;

  // Convert song ids to QVariant
  QVariantList songs_ids_qvariant;
  foreach (const int song_id, songs_ids_to_remove) {
    songs_ids_qvariant << QVariant(song_id);
  }
  QVariantList albums_ids_qvariant;
  QVariantList artists_ids_qvariant;

  parameters << Param("songIDs", songs_ids_qvariant);
  // We do not support albums and artist parameters for now, but they are
  // required
  parameters << Param("albumIDs", albums_ids_qvariant);
  parameters << Param("artistIDs", artists_ids_qvariant);

  QNetworkReply* reply = CreateRequest("removeUserLibrarySongs", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
    SLOT(SongsRemovedFromLibrary(QNetworkReply*, int)), reply, task_id);
}

void GroovesharkService::SongsRemovedFromLibrary(QNetworkReply* reply, int task_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark removeUserLibrarySongs failed";
    return;
  }
  RetrieveUserLibrarySongs();
}

QNetworkReply* GroovesharkService::CreateRequest(
    const QString& method_name,
    const QList<Param>& params,
    bool use_https) {

  QVariantMap request_params;
  request_params.insert("method", method_name);

  QVariantMap header;
  header.insert("wsKey", kApiKey);
  if (session_id_.isEmpty()) {
    if (method_name != "startSession") {
      // It's normal to not have a session_id when calling startSession.
      // Otherwise it's not, so print a warning message
      qLog(Warning) << "Session ID is empty: will not be added to query";
    }
  } else {
    header.insert("sessionID", session_id_);
  }
  request_params.insert("header", header);

  QVariantMap parameters;
  foreach(const Param& param, params) {
    parameters.insert(param.first, param.second);
  }
  request_params.insert("parameters", parameters);

  QJson::Serializer serializer;
  QByteArray post_params = serializer.serialize(request_params);

  QUrl url(kUrl);
  if (use_https) {
    url.setScheme("https");
  }
  url.setQueryItems( QList<QPair<QString, QString> >() << QPair<QString, QString>("sig", Utilities::HmacMd5(api_key_, post_params).toHex()));
  QNetworkRequest req(url);
  QNetworkReply *reply = network_->post(req, post_params);

  if (use_https) {
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            SLOT(RequestSslErrors(QList<QSslError>)));
  }

  return reply;
}

void GroovesharkService::RequestSslErrors(const QList<QSslError>& errors) {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

  foreach (const QSslError& error, errors) {
    emit StreamError("SSL error occurred in Grooveshark request for " +
                     reply->url().toString() + ": " + error.errorString());
  }
}

bool GroovesharkService::WaitForReply(QNetworkReply* reply) {
  QEventLoop event_loop;
  QTimer timeout_timer;
  connect(&timeout_timer, SIGNAL(timeout()), &event_loop, SLOT(quit()));
  connect(reply, SIGNAL(finished()), &event_loop, SLOT(quit()));
  timeout_timer.start(10000);
  event_loop.exec();
  if (!timeout_timer.isActive()) {
    qLog(Error) << "Grooveshark request timeout";
    return false;
  }
  timeout_timer.stop();
  return true;
}

QVariantMap GroovesharkService::ExtractResult(QNetworkReply* reply) {
  QJson::Parser parser;
  bool ok;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Error while parsing Grooveshark result";
  }
  QVariantList errors = result["errors"].toList();
  QVariantList::iterator it;
  for (it = errors.begin(); it != errors.end(); ++it) {
    QVariantMap error = (*it).toMap();
    qLog(Error) << "Grooveshark error: " << error["message"].toString();
    switch (error["code"].toInt()) {
      case 100: // User auth required
      case 102: // User premium required
        // These errors can happen if session_id is obsolete (e.g. we haven't use
        // it for more than two weeks): force the user to login again
        Logout(); 
        break;
    }
  }
  return result["result"].toMap();
}

SongList GroovesharkService::ExtractSongs(const QVariantMap& result) {
  QVariantList result_songs = result["songs"].toList();
  SongList songs;
  for (int i=0; i<result_songs.size(); ++i) {
    QVariantMap result_song = result_songs[i].toMap();
    songs << ExtractSong(result_song);
  }
  return songs;
}

Song GroovesharkService::ExtractSong(const QVariantMap& result_song) {
  Song song;
  if (!result_song.isEmpty()) {
    int song_id = result_song["SongID"].toInt();
    QString song_name = result_song["SongName"].toString();
    int artist_id = result_song["ArtistID"].toInt();
    QString artist_name = result_song["ArtistName"].toString();
    int album_id = result_song["AlbumID"].toInt();
    QString album_name = result_song["AlbumName"].toString();
    qint64 duration = result_song["EstimateDuration"].toInt() * kNsecPerSec;
    song.Init(song_name, artist_name, album_name, duration);
    QVariant cover = result_song["CoverArtFilename"];
    if (cover.isValid()) {
      song.set_art_automatic(QString(kUrlCover) + cover.toString());
    }
    QVariant track_number = result_song["TrackNum"];
    if (track_number.isValid()) {
      song.set_track(track_number.toInt());
    }
    QVariant year = result_song["Year"];
    if (year.isValid()) {
      song.set_year(year.toInt());
    }
    // Special kind of URL: because we need to request a stream key for each
    // play, we generate a fake URL for now, and we will create a real streaming
    // URL when user will actually play the song (through url handler)
    // URL is grooveshark://artist_id/album_id/song_id
    song.set_url(QUrl(QString("grooveshark://%1/%2/%3").arg(artist_id).arg(album_id).arg(song_id)));
  }
  return song;
}

QList<int> GroovesharkService::ExtractSongsIds(const QVariantMap& result) {
  QVariantList result_songs = result["songs"].toList();
  QList<int> songs_ids;
  for (int i=0; i<result_songs.size(); ++i) {
    QVariantMap result_song = result_songs[i].toMap();
    int song_id = result_song["SongID"].toInt();
    songs_ids << song_id;
  }
  return songs_ids;
}

QList<int> GroovesharkService::ExtractSongsIds(const QList<QUrl>& urls) {
  QList<int> songs_ids;
  foreach (const QUrl& url, urls) {
    int song_id = ExtractSongId(url);
    if (song_id) {
      songs_ids << song_id;
    }
  }
  return songs_ids;
}

int GroovesharkService::ExtractSongId(const QUrl& url) {
  if (url.scheme() == "grooveshark") {
    QStringList ids = url.toString().remove("grooveshark://").split("/");
    if (ids.size() == 3)
      // Returns the third id: song id
      return ids[2].toInt();
  }
  return 0;
}

QList<GroovesharkService::PlaylistInfo> GroovesharkService::ExtractPlaylistInfo(const QVariantMap& result) {
  QVariantList playlists_qvariant = result["playlists"].toList();

  QList<PlaylistInfo> playlists;

  // Get playlists info
  foreach (const QVariant& playlist_qvariant, playlists_qvariant) {
    QVariantMap playlist = playlist_qvariant.toMap();
    int playlist_id = playlist["PlaylistID"].toInt();
    QString playlist_name = playlist["PlaylistName"].toString();

    playlists << PlaylistInfo(playlist_id, playlist_name);
  }

  // Sort playlists by name
  qSort(playlists.begin(), playlists.end());

  return playlists;
}
