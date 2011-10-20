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

#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "qtiocompressor.h"

#include "internetmodel.h"
#include "groovesharksearchplaylisttype.h"
#include "groovesharkurlhandler.h"

#include "core/closure.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/scopedtransaction.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/groovesharksearchprovider.h"
#include "playlist/playlist.h"
#include "playlist/playlistcontainer.h"
#include "playlist/playlistmanager.h"
#include "ui/iconloader.h"

// The Grooveshark terms of service require that application keys are not
// accessible to third parties. Therefore this application key is obfuscated to
// prevent third parties from viewing it.
const char* GroovesharkService::kApiKey = "clementineplayer";
const char* GroovesharkService::kApiSecret = "MWVlNmU1N2IzNGY3MjA1ZTg1OWJkMTllNjk4YzEzZjY";

const char* GroovesharkService::kServiceName = "Grooveshark";
const char* GroovesharkService::kSettingsGroup = "Grooveshark";
const char* GroovesharkService::kUrl = "http://api.grooveshark.com/ws/3.0/";
const char* GroovesharkService::kUrlCover = "http://beta.grooveshark.com/static/amazonart/m";

const int GroovesharkService::kSearchDelayMsec = 400;
const int GroovesharkService::kSongSearchLimit = 50;

typedef QPair<QString, QVariant> Param;

GroovesharkService::GroovesharkService(InternetModel *parent)
  : InternetService(kServiceName, parent, parent),
    url_handler_(new GroovesharkUrlHandler(this, this)),
    pending_search_playlist_(NULL),
    next_pending_search_id_(0),
    root_(NULL),
    search_(NULL),
    network_(new NetworkAccessManager(this)),
    context_menu_(NULL),
    search_delay_(new QTimer(this)),
    last_search_reply_(NULL),
    api_key_(QByteArray::fromBase64(kApiSecret)),
    login_state_(LoginState_OtherError) {

  model()->player()->RegisterUrlHandler(url_handler_);
  model()->player()->playlists()->RegisterSpecialPlaylistType(new GroovesharkSearchPlaylistType(this));

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));

  // Get already existing (authenticated) session id, if any
  QSettings s;
  s.beginGroup(GroovesharkService::kSettingsGroup);
  session_id_ = s.value("sessionid").toString();
  username_ = s.value("username").toString();

  GroovesharkSearchProvider* search_provider = new GroovesharkSearchProvider(this);
  search_provider->Init(this);
  model()->global_search()->AddProvider(search_provider, false);
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
  emit OpenSettingsAtPage(SettingsDialog::Page_Grooveshark);
}

void GroovesharkService::Search(const QString& text, Playlist* playlist, bool now) {
  pending_search_ = text;
  pending_search_playlist_ = playlist;

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
             << Param("limit", QString::number(kSongSearchLimit))
             << Param("offset", "");

  QNetworkReply* reply = CreateRequest("getSongSearchResults", parameters, false);
  connect(reply, SIGNAL(finished()), SLOT(SimpleSearchFinished()));

  int id = next_pending_search_id_++;
  pending_searches_[reply] = id;

  return id;
}

void GroovesharkService::SimpleSearchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();

  const int id = pending_searches_.take(reply);
  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  emit SimpleSearchResults(id, songs);
}

int GroovesharkService::SearchAlbums(const QString& query) {
  QList<Param> parameters;
  parameters << Param("query", query)
             << Param("country", "")
             << Param("limit", QString::number(5));

  QNetworkReply* reply = CreateRequest("getAlbumSearchResults", parameters, false);

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

  SongList ret;
  foreach (const QVariant& v, albums) {
    QVariantMap album = v.toMap();
    quint64 album_id = album["AlbumID"].toULongLong();
    QString album_name = album["AlbumName"].toString();
    QString artist_name = album["ArtistName"].toString();
    QString cover_art = album["CoverArtFilename"].toString();

    qLog(Debug) << "Found:" << album_name << artist_name;
    Song song;
    song.Init(QString::null, album_name, artist_name, 0);
    song.set_art_automatic(QString(kUrlCover) + cover_art);
    QUrl url;
    url.setScheme("grooveshark");
    url.setPath(QString("album/%1").arg(album_id));
    song.set_url(url);

    ret << song;
  }

  emit AlbumSearchResult(id, ret);
}

void GroovesharkService::FetchSongsForAlbum(int id, const QUrl& url) {
  QStringList split = url.path().split('/');
  Q_ASSERT(split.length() == 2);
  FetchSongsForAlbum(id, split[1].toULongLong());
}

void GroovesharkService::FetchSongsForAlbum(int id, quint64 album_id) {
  QList<Param> parameters;
  parameters << Param("albumID", album_id)
             << Param("country", "");

  QNetworkReply* reply = CreateRequest("getAlbumSongs", parameters, false);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(GetAlbumSongsFinished(QNetworkReply*,int)),
             reply, id);
}

void GroovesharkService::GetAlbumSongsFinished(
    QNetworkReply* reply, int id) {
  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);

  emit AlbumSongsLoaded(id, songs);
}

void GroovesharkService::DoSearch() {
  QList<Param> parameters;

  parameters  << Param("query", pending_search_)
              << Param("country", "")
              << Param("limit", QString("%1").arg(kSongSearchLimit))
              << Param("offset", "");
  last_search_reply_ = CreateRequest("getSongSearchResults", parameters, false);
  connect(last_search_reply_, SIGNAL(finished()), SLOT(SearchSongsFinished()));
}

void GroovesharkService::SearchSongsFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply || reply != last_search_reply_)
    return;

  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  pending_search_playlist_->Clear();
  pending_search_playlist_->InsertSongs(songs);
}

void GroovesharkService::InitCountry() {
  if (!country_.isEmpty())
    return;
  // Get country info
  QNetworkReply *reply_country = CreateRequest("getCountry", QList<Param>(), true);

  // Wait for the reply
  {
    QEventLoop event_loop;
    QTimer timeout_timer;
    connect(&timeout_timer, SIGNAL(timeout()), &event_loop, SLOT(quit()));
    connect(reply_country, SIGNAL(finished()), &event_loop, SLOT(quit()));
    timeout_timer.start(3000);
    event_loop.exec();
    if (!timeout_timer.isActive()) {
      qLog(Error) << "Grooveshark request timeout";
      return;
    }
    timeout_timer.stop();
  }
  country_ = ExtractResult(reply_country);
}

QUrl GroovesharkService::GetStreamingUrlFromSongId(const QString& song_id,
    QString* server_id, QString* stream_key, qint64* length_nanosec) {
  QList<Param> parameters;

  InitCountry();
  parameters  << Param("songID", song_id)
              << Param("country", country_);
  QNetworkReply* reply = CreateRequest("getSubscriberStreamKey", parameters, true);
  // Wait for the reply
  {
    QEventLoop event_loop;
    QTimer timeout_timer;
    connect(&timeout_timer, SIGNAL(timeout()), &event_loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &event_loop, SLOT(quit()));
    timeout_timer.start(3000);
    event_loop.exec();
    if (!timeout_timer.isActive()) {
      qLog(Error) << "Grooveshark request timeout";
      return QUrl();
    }
    timeout_timer.stop();
  }
  QVariantMap result = ExtractResult(reply);
  server_id->clear();
  server_id->append(result["StreamServerID"].toString());
  stream_key->clear();
  stream_key->append(result["StreamKey"].toString());
  *length_nanosec = result["uSecs"].toLongLong() * 1000;

  return QUrl(result["url"].toString());
}

void GroovesharkService::Login(const QString& username, const QString& password) {
  // To login, we first need to create a session. Next, we will authenticate
  // this session using the user's username and password (for now, we just keep
  // them in mind)
  username_ = username;
  password_ = QCryptographicHash::hash(password.toLocal8Bit(), QCryptographicHash::Md5).toHex();

  QList<Param> parameters;
  QNetworkReply *reply = CreateRequest("startSession", parameters, false, true);

  connect(reply, SIGNAL(finished()), SLOT(SessionCreated()));
}

void GroovesharkService::SessionCreated() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

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

  QNetworkReply *reply = CreateRequest("authenticate", parameters, true, true);
  connect(reply, SIGNAL(finished()), SLOT(Authenticated()));
}

void GroovesharkService::Authenticated() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

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

void GroovesharkService::Logout() {
  ResetSessionId();
  root_->removeRows(0, root_->rowCount());
  // search item was root's child, and has been deleted: we should update this
  // now invalid pointer
  search_ = NULL;
}

void GroovesharkService::ResetSessionId() {
  QSettings s;
  s.beginGroup(GroovesharkService::kSettingsGroup);

  session_id_.clear();
  s.setValue("sessionid", session_id_);
}

void GroovesharkService::ShowContextMenu(const QModelIndex& index, const QPoint& global_pos) {
  EnsureMenuCreated();
  context_menu_->popup(global_pos);
  context_item_ = index;
}

QModelIndex GroovesharkService::GetCurrentIndex() {
  return context_item_;
}

void GroovesharkService::UpdateTotalSongCount(int count) {
}

void GroovesharkService::EnsureMenuCreated() {
  if(!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("edit-find"), tr("Search Grooveshark (opens a new tab)") + "...", this, SLOT(OpenSearchTab()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure"), tr("Configure Grooveshark..."), this, SLOT(ShowConfig()));
  }
}

void GroovesharkService::EnsureItemsCreated() {
  if (IsLoggedIn() && !search_) {
    search_ = new QStandardItem(IconLoader::Load("edit-find"),
                                tr("Search Grooveshark (opens a new tab)"));
    search_->setData(Type_SearchResults, InternetModel::Role_Type);
    search_->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                             InternetModel::Role_PlayBehaviour);
    root_->appendRow(search_);
    RetrieveUserPlaylists();
  }
}

void GroovesharkService::EnsureConnected() {
  if (session_id_.isEmpty()) {
    ShowConfig();
  } else {
    EnsureItemsCreated();
  }
}

void GroovesharkService::RetrieveUserPlaylists() {
  QNetworkReply* reply = CreateRequest("getUserPlaylists", QList<Param>(), true);

  connect(reply, SIGNAL(finished()), SLOT(UserPlaylistsRetrieved()));
}

void GroovesharkService::UserPlaylistsRetrieved() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  QVariantMap result = ExtractResult(reply);
  QVariantList playlists = result["playlists"].toList();
  QVariantList::iterator it;
  for (it = playlists.begin(); it != playlists.end(); ++it) {
    // Get playlist info
    QVariantMap playlist = (*it).toMap();
    int playlist_id = playlist["PlaylistID"].toInt();
    QString playlist_name = playlist["PlaylistName"].toString();

    // Request playlist's songs
    QNetworkReply *reply;
    QList<Param> parameters;
    parameters << Param("playlistID", playlist_id);
    reply = CreateRequest("getPlaylistSongs", parameters, true); 
    connect(reply, SIGNAL(finished()), SLOT(PlaylistSongsRetrieved()));
    
    // Keep in mind correspondance between reply object and playlist
    pending_retrieve_playlists_.insert(reply, PlaylistInfo(playlist_id, playlist_name));
  }
}

void GroovesharkService::PlaylistSongsRetrieved() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  // Find corresponding playlist info
  PlaylistInfo playlist_info = pending_retrieve_playlists_.take(reply);
  // Create playlist item
  QStandardItem* item = new QStandardItem(playlist_info.name_);
  item->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);

  QVariantMap result = ExtractResult(reply);
  SongList songs = ExtractSongs(result);
  foreach (const Song& song, songs) {
    QStandardItem* child = new QStandardItem(song.PrettyTitleWithArtist());
    child->setData(Type_Track, InternetModel::Role_Type);
    child->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    child->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
    child->setData(song.url(), InternetModel::Role_Url);

    item->appendRow(child);
  }
  root_->appendRow(item);
}

void GroovesharkService::MarkStreamKeyOver30Secs(const QString& stream_key,
                                                 const QString& server_id) {
  QList<Param> parameters;
  parameters  << Param("streamKey", stream_key)
              << Param("streamServerID", server_id);

  QNetworkReply* reply = CreateRequest("markStreamKeyOver30Secs", parameters, true, false);
  connect(reply, SIGNAL(finished()), SLOT(StreamMarked()));
}

void GroovesharkService::StreamMarked() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

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

  QNetworkReply* reply = CreateRequest("markSongComplete", parameters, true, false);
  connect(reply, SIGNAL(finished()), SLOT(SongMarkedAsComplete()));
}

void GroovesharkService::SongMarkedAsComplete() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();
  QVariantMap result = ExtractResult(reply);
  if (!result["success"].toBool()) {
    qLog(Warning) << "Grooveshark markSongComplete failed";
  }
}

void GroovesharkService::OpenSearchTab() {
  model()->player()->playlists()->New(tr("Search Grooveshark"), SongList(),
                                      GroovesharkSearchPlaylistType::kName);
}

void GroovesharkService::ItemDoubleClicked(QStandardItem* item) {
  if (item == search_) {
    OpenSearchTab();
  }
  if (item == root_) {
    EnsureConnected();
  }
}

QNetworkReply* GroovesharkService::CreateRequest(const QString& method_name, QList<Param> params,
                                       bool need_authentication,
                                       bool use_https) {
  QVariantMap request_params;
  request_params.insert("method", method_name);

  QVariantMap header;
  header.insert("wsKey", kApiKey);
  if (need_authentication) {
    if (session_id_.isEmpty()) {
      qLog(Warning) << "Session ID is empty: will not be added to query";
    } else {
      header.insert("sessionID", session_id_);
    }
  }
  request_params.insert("header", header);

  QVariantMap parameters;
  foreach(const Param& param, params) {
    parameters.insert(param.first, param.second);
  }
  request_params.insert("parameters", parameters);

  QJson::Serializer serializer;
  QByteArray post_params = serializer.serialize(request_params);

  qLog(Debug) << post_params;

  QUrl url(kUrl);
  if (use_https) {
    url.setScheme("https");
  }
  url.setQueryItems( QList<QPair<QString, QString> >() << QPair<QString, QString>("sig", Utilities::HmacMd5(api_key_, post_params).toHex()));
  QNetworkRequest req(url);
  QNetworkReply *reply = network_->post(req, post_params);

  return reply;
}

QVariantMap GroovesharkService::ExtractResult(QNetworkReply* reply) {
  QJson::Parser parser;
  bool ok;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Error while parsing Grooveshark result";
  }
  qLog(Debug) << result;
  return result["result"].toMap();
}

SongList GroovesharkService::ExtractSongs(const QVariantMap& result) {
  QVariantList result_songs = result["songs"].toList();
  SongList songs;
  for (int i=0; i<result_songs.size(); ++i) {
    QVariantMap result_song = result_songs[i].toMap();
    Song song;
    int song_id = result_song["SongID"].toInt();
    QString song_name = result_song["SongName"].toString();
    QString artist_name = result_song["ArtistName"].toString();
    QString album_name = result_song["AlbumName"].toString();
    QString cover = result_song["CoverArtFilename"].toString();
    song.Init(song_name, artist_name, album_name, 0);
    song.set_art_automatic(QString(kUrlCover) + cover);
    // Special kind of URL: because we need to request a stream key for each
    // play, we generate a fake URL for now, and we will create a real streaming
    // URL when user will actually play the song (through url handler)
    song.set_url(QString("grooveshark://%1").arg(song_id));
    songs << song;
  }
  return songs;
}
