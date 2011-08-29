#include "config.h"
#include "internetmodel.h"
#include "spotifyblobdownloader.h"
#include "spotifyserver.h"
#include "spotifyservice.h"
#include "spotifysearchplaylisttype.h"
#include "spotifyurlhandler.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "playlist/playlist.h"
#include "playlist/playlistcontainer.h"
#include "playlist/playlistmanager.h"
#include "spotifyblob/common/blobversion.h"
#include "spotifyblob/common/spotifymessagehandler.h"
#include "widgets/didyoumean.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QVariant>

Q_DECLARE_METATYPE(QStandardItem*);

const char* SpotifyService::kServiceName = "Spotify";
const char* SpotifyService::kSettingsGroup = "Spotify";
const char* SpotifyService::kBlobDownloadUrl = "http://spotify.clementine-player.org/";
const int SpotifyService::kSearchDelayMsec = 400;

SpotifyService::SpotifyService(InternetModel* parent)
    : InternetService(kServiceName, parent, parent),
      server_(NULL),
      url_handler_(new SpotifyUrlHandler(this, this)),
      blob_process_(NULL),
      root_(NULL),
      search_(NULL),
      starred_(NULL),
      inbox_(NULL),
      login_task_id_(0),
      pending_search_playlist_(NULL),
      context_menu_(NULL),
      search_delay_(new QTimer(this)),
      login_state_(LoginState_OtherError) {
  // Build the search path for the binary blob.
  // Look for one distributed alongside clementine first, then check in the
  // user's home directory for any that have been downloaded.
#ifdef Q_OS_MAC
  system_blob_path_ = QCoreApplication::applicationDirPath() +
      "/../PlugIns/clementine-spotifyblob";
#else
  system_blob_path_ = QCoreApplication::applicationDirPath() +
      "/clementine-spotifyblob" CMAKE_EXECUTABLE_SUFFIX;
#endif

  local_blob_version_ = QString("version%1-%2bit").arg(SPOTIFY_BLOB_VERSION).arg(sizeof(void*) * 8);
  local_blob_path_    = Utilities::GetConfigPath(Utilities::Path_LocalSpotifyBlob) +
                        "/" + local_blob_version_ + "/blob";

  qLog(Debug) << "Spotify system blob path:" << system_blob_path_;
  qLog(Debug) << "Spotify local blob path:" << local_blob_path_;

  model()->player()->RegisterUrlHandler(url_handler_);
  model()->player()->playlists()->RegisterSpecialPlaylistType(
        new SpotifySearchPlaylistType(this));

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));
}

SpotifyService::~SpotifyService() {
  if (blob_process_ && blob_process_->state() == QProcess::Running) {
    qLog(Info) << "Terminating blob process...";
    blob_process_->terminate();
    blob_process_->waitForFinished(1000);
  }
}

QStandardItem* SpotifyService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":icons/svg/spotify.svg"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SpotifyService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      EnsureServerCreated();
      break;

    case Type_SearchResults:
      break;

    case Type_InboxPlaylist:
      EnsureServerCreated();
      server_->LoadInbox();
      break;

    case Type_StarredPlaylist:
      EnsureServerCreated();
      server_->LoadStarred();
      break;

    case Type_UserPlaylist:
      EnsureServerCreated();
      server_->LoadUserPlaylist(item->data(Role_UserPlaylistIndex).toInt());
      break;

    default:
      break;
  }

  return;
}

QModelIndex SpotifyService::GetCurrentIndex() {
  return QModelIndex();
}

void SpotifyService::Login(const QString& username, const QString& password) {
  Logout();
  EnsureServerCreated(username, password);
}

void SpotifyService::LoginCompleted(bool success, const QString& error,
                                    spotify_pb::LoginResponse_Error error_code) {
  if (login_task_id_) {
    model()->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  login_state_ = LoginState_LoggedIn;

  if (!success) {
    QMessageBox::warning(NULL, tr("Spotify login error"), error, QMessageBox::Close);

    switch (error_code) {
    case spotify_pb::LoginResponse_Error_BadUsernameOrPassword:
      login_state_ = LoginState_BadCredentials;
      break;

    case spotify_pb::LoginResponse_Error_UserBanned:
      login_state_ = LoginState_Banned;
      break;

    case spotify_pb::LoginResponse_Error_UserNeedsPremium:
      login_state_ = LoginState_NoPremium;
      break;

    default:
      login_state_ = LoginState_OtherError;
    }
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("login_state", login_state_);

  emit LoginFinished(success);
}

void SpotifyService::BlobProcessError(QProcess::ProcessError error) {
  qLog(Error) << "Spotify blob process failed:" << error;
  blob_process_->deleteLater();
  blob_process_ = NULL;

  if (login_task_id_) {
    model()->task_manager()->SetTaskFinished(login_task_id_);
  }
}

void SpotifyService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  login_state_ = LoginState(s.value("login_state", LoginState_OtherError).toInt());
}

void SpotifyService::EnsureServerCreated(const QString& username,
                                         const QString& password) {
  if (server_ && blob_process_) {
    return;
  }

  delete server_;
  server_ = new SpotifyServer(this);

  connect(server_, SIGNAL(LoginCompleted(bool,QString,spotify_pb::LoginResponse_Error)),
                   SLOT(LoginCompleted(bool,QString,spotify_pb::LoginResponse_Error)));
  connect(server_, SIGNAL(PlaylistsUpdated(spotify_pb::Playlists)),
          SLOT(PlaylistsUpdated(spotify_pb::Playlists)));
  connect(server_, SIGNAL(InboxLoaded(spotify_pb::LoadPlaylistResponse)),
          SLOT(InboxLoaded(spotify_pb::LoadPlaylistResponse)));
  connect(server_, SIGNAL(StarredLoaded(spotify_pb::LoadPlaylistResponse)),
          SLOT(StarredLoaded(spotify_pb::LoadPlaylistResponse)));
  connect(server_, SIGNAL(UserPlaylistLoaded(spotify_pb::LoadPlaylistResponse)),
          SLOT(UserPlaylistLoaded(spotify_pb::LoadPlaylistResponse)));
  connect(server_, SIGNAL(PlaybackError(QString)),
          SIGNAL(StreamError(QString)));
  connect(server_, SIGNAL(SearchResults(spotify_pb::SearchResponse)),
          SLOT(SearchResults(spotify_pb::SearchResponse)));
  connect(server_, SIGNAL(ImageLoaded(QString,QImage)),
          SLOT(ImageLoaded(QString,QImage)));
  connect(server_, SIGNAL(SyncPlaylistProgress(spotify_pb::SyncPlaylistProgress)),
          SLOT(SyncPlaylistProgress(spotify_pb::SyncPlaylistProgress)));

  server_->Init();

  login_task_id_ = model()->task_manager()->StartTask(tr("Connecting to Spotify"));

  if (username.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);

    server_->Login(s.value("username").toString(), s.value("password").toString());
  } else {
    server_->Login(username, password);
  }

  StartBlobProcess();
}

void SpotifyService::StartBlobProcess() {
  // Try to find an executable to run
  QString blob_path;
  QProcessEnvironment env(QProcessEnvironment::systemEnvironment());

  // Look in the system search path first
  if (QFile::exists(system_blob_path_)) {
    blob_path = system_blob_path_;
  }

  // Next look in the local path
  if (blob_path.isEmpty()) {
    if (QFile::exists(local_blob_path_)) {
      blob_path = local_blob_path_;
      env.insert("LD_LIBRARY_PATH", QFileInfo(local_blob_path_).path());
    }
  }

  if (blob_path.isEmpty()) {
    // If the blob still wasn't found then we'll prompt the user to download one
    if (login_task_id_) {
      model()->task_manager()->SetTaskFinished(login_task_id_);
    }

    #ifdef Q_OS_LINUX
      if (SpotifyBlobDownloader::Prompt()) {
        InstallBlob();
      }
    #endif

    return;
  }

  delete blob_process_;
  blob_process_ = new QProcess(this);
  blob_process_->setProcessChannelMode(QProcess::ForwardedChannels);
  blob_process_->setProcessEnvironment(env);

  connect(blob_process_,
          SIGNAL(error(QProcess::ProcessError)),
          SLOT(BlobProcessError(QProcess::ProcessError)));

  qLog(Info) << "Starting" << blob_path;
  blob_process_->start(
        blob_path, QStringList() << QString::number(server_->server_port()));
}

bool SpotifyService::IsBlobInstalled() const {
  return QFile::exists(system_blob_path_) ||
         QFile::exists(local_blob_path_);
}

void SpotifyService::InstallBlob() {
  // The downloader deletes itself when it finishes
  SpotifyBlobDownloader* downloader = new SpotifyBlobDownloader(
        local_blob_version_, QFileInfo(local_blob_path_).path(), this);
  connect(downloader, SIGNAL(Finished()), SLOT(BlobDownloadFinished()));
  connect(downloader, SIGNAL(Finished()), SIGNAL(BlobStateChanged()));
  downloader->Start();
}

void SpotifyService::BlobDownloadFinished() {
  EnsureServerCreated();
}

void SpotifyService::PlaylistsUpdated(const spotify_pb::Playlists& response) {
  if (login_task_id_) {
    model()->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  // Create starred and inbox playlists if they're not here already
  if (!search_) {
    search_ = new QStandardItem(IconLoader::Load("edit-find"),
                                tr("Search Spotify (opens a new tab)"));
    search_->setData(Type_SearchResults, InternetModel::Role_Type);
    search_->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                             InternetModel::Role_PlayBehaviour);

    starred_ = new QStandardItem(QIcon(":/star-on.png"), tr("Starred"));
    starred_->setData(Type_StarredPlaylist, InternetModel::Role_Type);
    starred_->setData(true, InternetModel::Role_CanLazyLoad);

    inbox_ = new QStandardItem(IconLoader::Load("mail-message"), tr("Inbox"));
    inbox_->setData(Type_InboxPlaylist, InternetModel::Role_Type);
    inbox_->setData(true, InternetModel::Role_CanLazyLoad);

    root_->appendRow(search_);
    root_->appendRow(starred_);
    root_->appendRow(inbox_);
  }

  // Don't do anything if the playlists haven't changed since last time.
  if (!DoPlaylistsDiffer(response)) {
    qLog(Debug) << "Playlists haven't changed - not updating";
    return;
  }

  // Remove and recreate the other playlists
  foreach (QStandardItem* item, playlists_) {
    item->parent()->removeRow(item->row());
  }
  playlists_.clear();

  for (int i=0 ; i<response.playlist_size() ; ++i) {
    const spotify_pb::Playlists::Playlist& msg = response.playlist(i);

    QStandardItem* item = new QStandardItem(QStringFromStdString(msg.name()));
    item->setData(Type_UserPlaylist, InternetModel::Role_Type);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    item->setData(msg.index(), Role_UserPlaylistIndex);

    root_->appendRow(item);
    playlists_ << item;
  }
}

bool SpotifyService::DoPlaylistsDiffer(const spotify_pb::Playlists& response) {
  if (playlists_.count() != response.playlist_size()) {
    return true;
  }

  for (int i=0 ; i<response.playlist_size() ; ++i) {
    const spotify_pb::Playlists::Playlist& msg = response.playlist(i);
    const QStandardItem* item = PlaylistBySpotifyIndex(msg.index());

    if (!item) {
      return true;
    }

    if (QStringFromStdString(msg.name()) != item->text()) {
      return true;
    }
  }

  return false;
}

void SpotifyService::InboxLoaded(const spotify_pb::LoadPlaylistResponse& response) {
  FillPlaylist(inbox_, response);
}

void SpotifyService::StarredLoaded(const spotify_pb::LoadPlaylistResponse& response) {
  FillPlaylist(starred_, response);
}

QStandardItem* SpotifyService::PlaylistBySpotifyIndex(int index) const {
  foreach (QStandardItem* item, playlists_) {
    if (item->data(Role_UserPlaylistIndex).toInt() == index) {
      return item;
    }
  }
  return NULL;
}

void SpotifyService::UserPlaylistLoaded(const spotify_pb::LoadPlaylistResponse& response) {
  // Find a playlist with this index
  QStandardItem* item = PlaylistBySpotifyIndex(response.request().user_playlist_index());
  if (item) {
    FillPlaylist(item, response);
  }
}

void SpotifyService::FillPlaylist(QStandardItem* item, const spotify_pb::LoadPlaylistResponse& response) {
  if (item->hasChildren())
    item->removeRows(0, item->rowCount());

  for (int i=0 ; i<response.track_size() ; ++i) {
    Song song;
    SongFromProtobuf(response.track(i), &song);

    QStandardItem* child = new QStandardItem(song.PrettyTitleWithArtist());
    child->setData(Type_Track, InternetModel::Role_Type);
    child->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    child->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
    child->setData(song.url(), InternetModel::Role_Url);

    item->appendRow(child);
  }
}

void SpotifyService::SongFromProtobuf(const spotify_pb::Track& track, Song* song) {
  song->set_rating(track.starred() ? 1.0 : 0.0);
  song->set_title(QStringFromStdString(track.title()));
  song->set_album(QStringFromStdString(track.album()));
  song->set_length_nanosec(track.duration_msec() * kNsecPerMsec);
  song->set_score(track.popularity());
  song->set_disc(track.disc());
  song->set_track(track.track());
  song->set_year(track.year());
  song->set_url(QUrl(QStringFromStdString(track.uri())));
  song->set_art_automatic("spotify://image/" + QStringFromStdString(track.album_art_id()));

  QStringList artists;
  for (int i=0 ; i<track.artist_size() ; ++i) {
    artists << QStringFromStdString(track.artist(i));
  }

  song->set_artist(artists.join(", "));

  song->set_filetype(Song::Type_Stream);
  song->set_valid(true);
  song->set_directory_id(0);
  song->set_mtime(0);
  song->set_ctime(0);
  song->set_filesize(0);
}

PlaylistItem::Options SpotifyService::playlistitem_options() const {
  return PlaylistItem::PauseDisabled;
}

void SpotifyService::EnsureMenuCreated() {
  if (context_menu_)
    return;

  context_menu_ = new QMenu;

  context_menu_->addActions(GetPlaylistActions());
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("edit-find"), tr("Search Spotify (opens a new tab)..."), this, SLOT(OpenSearchTab()));
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("configure"), tr("Configure Spotify..."), this, SLOT(ShowConfig()));

  playlist_context_menu_ = new QMenu;
  playlist_sync_action_ = playlist_context_menu_->addAction(
      IconLoader::Load("view-refresh"),
      tr("Make playlist available offline"),
      this,
      SLOT(SyncPlaylist()));
}

void SpotifyService::SyncPlaylist() {
  QStandardItem* item = playlist_sync_action_->data().value<QStandardItem*>();
  Q_ASSERT(item);

  Type type = static_cast<Type>(item->data(InternetModel::Role_Type).toInt());
  switch (type) {
    case Type_UserPlaylist: {
      int index = item->data(Role_UserPlaylistIndex).toInt();
      server_->SyncUserPlaylist(index);
      playlist_sync_ids_[index] =
          model()->task_manager()->StartTask(tr("Syncing Spotify playlist"));
      break;
    }
    case Type_InboxPlaylist:
      server_->SyncInbox();
      inbox_sync_id_ = model()->task_manager()->StartTask(tr("Syncing Spotify inbox"));
      break;
    case Type_StarredPlaylist:
      server_->SyncStarred();
      starred_sync_id_ = model()->task_manager()->StartTask(tr("Syncing Spotify starred tracks"));
      break;
    default:
      break;
  }
}

void SpotifyService::Search(const QString& text, Playlist* playlist, bool now) {
  EnsureServerCreated();

  pending_search_ = text;
  pending_search_playlist_ = playlist;

  if (now) {
    search_delay_->stop();
    DoSearch();
  } else {
    search_delay_->start();
  }
}

void SpotifyService::DoSearch() {
  if (!pending_search_.isEmpty()) {
    server_->Search(pending_search_, 200);
  }
}

void SpotifyService::SearchResults(const spotify_pb::SearchResponse& response) {
  if (QStringFromStdString(response.request().query()) != pending_search_) {
    qLog(Debug) << "Old search result for"
                << QStringFromStdString(response.request().query())
                << "expecting" << pending_search_;
    return;
  }
  pending_search_.clear();

  SongList songs;
  for (int i=0 ; i<response.result_size() ; ++i) {
    Song song;
    SongFromProtobuf(response.result(i), &song);
    songs << song;
  }

  qLog(Debug) << "Got" << songs.count() << "results";

  pending_search_playlist_->Clear();
  pending_search_playlist_->InsertSongs(songs);

  const QString did_you_mean = QStringFromStdString(response.did_you_mean());
  if (!did_you_mean.isEmpty()) {
    model()->player()->playlists()->playlist_container()->did_you_mean()->Show(did_you_mean);
  }
}

SpotifyServer* SpotifyService::server() const {
  const_cast<SpotifyService*>(this)->EnsureServerCreated();
  return server_;
}

void SpotifyService::ShowContextMenu(const QModelIndex& index, const QPoint& global_pos) {
  EnsureMenuCreated();
  QStandardItem* item = model()->itemFromIndex(index);
  if (item) {
    Type type = static_cast<Type>(item->data(InternetModel::Role_Type).toInt());
    if (type == Type_InboxPlaylist ||
        type == Type_StarredPlaylist ||
        type == Type_UserPlaylist) {
      playlist_sync_action_->setData(qVariantFromValue(item));
      playlist_context_menu_->popup(global_pos);
      return;
    }
  }

  context_menu_->popup(global_pos);
}

void SpotifyService::OpenSearchTab() {
  model()->player()->playlists()->New(tr("Search Spotify"), SongList(),
                                      SpotifySearchPlaylistType::kName);
}

void SpotifyService::ItemDoubleClicked(QStandardItem* item) {
  if (item == search_) {
    OpenSearchTab();
  }
}

void SpotifyService::LoadImage(const QUrl& url) {
  if (url.scheme() != "spotify" || url.host() != "image") {
    return;
  }

  QString image_id = url.path();
  if (image_id.startsWith('/')) {
    image_id.remove(0, 1);
  }

  EnsureServerCreated();
  server_->LoadImage(image_id);
}

void SpotifyService::ImageLoaded(const QString& id, const QImage& image) {
  qLog(Debug) << "Image loaded:" << id;
  emit ImageLoaded(QUrl("spotify://image/" + id), image);
}

void SpotifyService::SyncPlaylistProgress(
    const spotify_pb::SyncPlaylistProgress& progress) {
  qLog(Debug) << "Sync progress:" << progress.sync_progress();
  int task_id = -1;
  switch (progress.request().type()) {
    case spotify_pb::Inbox:
      task_id = inbox_sync_id_;
      break;
    case spotify_pb::Starred:
      task_id = starred_sync_id_;
      break;
    case spotify_pb::UserPlaylist: {
      QMap<int, int>::const_iterator it = playlist_sync_ids_.constFind(
          progress.request().user_playlist_index());
      if (it != playlist_sync_ids_.constEnd()) {
        task_id = it.value();
      }
      break;
    }
    default:
      break;
  }
  if (task_id == -1) {
    qLog(Warning) << "Received sync progress for unknown playlist";
    return;
  }
  model()->task_manager()->SetTaskProgress(task_id, progress.sync_progress(), 100);
  if (progress.sync_progress() == 100) {
    model()->task_manager()->SetTaskFinished(task_id);
    if (progress.request().type() == spotify_pb::UserPlaylist) {
      playlist_sync_ids_.remove(task_id);
    }
  }
}

void SpotifyService::ShowConfig() {
  emit OpenSettingsAtPage(SettingsDialog::Page_Spotify);
}

void SpotifyService::Logout() {
  delete server_;
  delete blob_process_;
  server_ = NULL;
  blob_process_ = NULL;

  login_state_ = LoginState_OtherError;
}
