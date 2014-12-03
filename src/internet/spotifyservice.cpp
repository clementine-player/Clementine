#include "blobversion.h"
#include "config.h"
#include "internetmodel.h"
#include "searchboxwidget.h"
#include "spotifyserver.h"
#include "spotifyservice.h"
#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/spotifysearchprovider.h"
#include "playlist/playlist.h"
#include "playlist/playlistcontainer.h"
#include "playlist/playlistmanager.h"
#include "ui/iconloader.h"
#include "widgets/didyoumean.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QSettings>
#include <QVariant>

#ifdef HAVE_SPOTIFY_DOWNLOADER
#include "spotifyblobdownloader.h"
#endif

Q_DECLARE_METATYPE(QStandardItem*);

const char* SpotifyService::kServiceName = "Spotify";
const char* SpotifyService::kSettingsGroup = "Spotify";
const char* SpotifyService::kBlobDownloadUrl =
    "http://spotify.clementine-player.org/";
const int SpotifyService::kSearchDelayMsec = 400;

SpotifyService::SpotifyService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      server_(nullptr),
      blob_process_(nullptr),
      root_(nullptr),
      search_(nullptr),
      starred_(nullptr),
      inbox_(nullptr),
      toplist_(nullptr),
      login_task_id_(0),
      context_menu_(nullptr),
      playlist_context_menu_(nullptr),
      song_context_menu_(nullptr),
      playlist_sync_action_(nullptr),
      remove_from_playlist_(nullptr),
      search_box_(new SearchBoxWidget(this)),
      search_delay_(new QTimer(this)),
      login_state_(LoginState_OtherError),
      bitrate_(pb::spotify::Bitrate320k),
      volume_normalisation_(false) {
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

  local_blob_version_ = QString("version%1-%2bit")
                            .arg(SPOTIFY_BLOB_VERSION)
                            .arg(sizeof(void*) * 8);
  local_blob_path_ =
      Utilities::GetConfigPath(Utilities::Path_LocalSpotifyBlob) + "/" +
      local_blob_version_ + "/blob";

  qLog(Debug) << "Spotify system blob path:" << system_blob_path_;
  qLog(Debug) << "Spotify local blob path:" << local_blob_path_;

  app_->global_search()->AddProvider(new SpotifySearchProvider(app_, this));

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));
  connect(search_box_, SIGNAL(TextChanged(QString)), SLOT(Search(QString)));
}

SpotifyService::~SpotifyService() {
  if (blob_process_ && blob_process_->state() == QProcess::Running) {
    qLog(Info) << "Terminating blob process...";
    blob_process_->terminate();
    blob_process_->waitForFinished(1000);
  }
}

QStandardItem* SpotifyService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":icons/22x22/spotify.png"), kServiceName);
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

    case InternetModel::Type_UserPlaylist:
      EnsureServerCreated();
      server_->LoadUserPlaylist(item->data(Role_UserPlaylistIndex).toInt());
      break;

    case Type_Toplist:
      EnsureServerCreated();
      server_->LoadToplist();
      break;

    default:
      break;
  }

  return;
}

void SpotifyService::Login(const QString& username, const QString& password) {
  Logout();
  EnsureServerCreated(username, password);
}

void SpotifyService::LoginCompleted(
    bool success, const QString& error,
    pb::spotify::LoginResponse_Error error_code) {
  if (login_task_id_) {
    app_->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  if (!success) {
    bool show_error_dialog = true;
    QString error_copy(error);

    switch (error_code) {
      case pb::spotify::LoginResponse_Error_BadUsernameOrPassword:
        login_state_ = LoginState_BadCredentials;
        break;

      case pb::spotify::LoginResponse_Error_UserBanned:
        login_state_ = LoginState_Banned;
        break;

      case pb::spotify::LoginResponse_Error_UserNeedsPremium:
        login_state_ = LoginState_NoPremium;
        break;

      case pb::spotify::LoginResponse_Error_ReloginFailed:
        if (login_state_ == LoginState_LoggedIn) {
          // This is the first time the relogin has failed - show a message this
          // time only.
          error_copy =
              tr("You have been logged out of Spotify, please re-enter your "
                 "password in the Settings dialog.");
        } else {
          show_error_dialog = false;
        }

        login_state_ = LoginState_ReloginFailed;
        break;

      default:
        login_state_ = LoginState_OtherError;
        break;
    }

    if (show_error_dialog) {
      QMessageBox::warning(nullptr, tr("Spotify login error"), error_copy,
                           QMessageBox::Close);
    }
  } else {
    login_state_ = LoginState_LoggedIn;
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("login_state", login_state_);

  emit LoginFinished(success);
}

void SpotifyService::BlobProcessError(QProcess::ProcessError error) {
  qLog(Error) << "Spotify blob process failed:" << error;
  blob_process_->deleteLater();
  blob_process_ = nullptr;

  if (login_task_id_) {
    app_->task_manager()->SetTaskFinished(login_task_id_);
  }
}

void SpotifyService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  login_state_ =
      LoginState(s.value("login_state", LoginState_OtherError).toInt());
  bitrate_ = static_cast<pb::spotify::Bitrate>(
      s.value("bitrate", pb::spotify::Bitrate320k).toInt());
  volume_normalisation_ = s.value("volume_normalisation", false).toBool();

  if (server_ && blob_process_) {
    server_->SetPlaybackSettings(bitrate_, volume_normalisation_);
  }
}

void SpotifyService::EnsureServerCreated(const QString& username,
                                         const QString& password) {
  if (server_ && blob_process_) {
    return;
  }

  delete server_;
  server_ = new SpotifyServer(this);

  connect(
      server_,
      SIGNAL(LoginCompleted(bool, QString, pb::spotify::LoginResponse_Error)),
      SLOT(LoginCompleted(bool, QString, pb::spotify::LoginResponse_Error)));
  connect(server_, SIGNAL(PlaylistsUpdated(pb::spotify::Playlists)),
          SLOT(PlaylistsUpdated(pb::spotify::Playlists)));
  connect(server_, SIGNAL(InboxLoaded(pb::spotify::LoadPlaylistResponse)),
          SLOT(InboxLoaded(pb::spotify::LoadPlaylistResponse)));
  connect(server_, SIGNAL(StarredLoaded(pb::spotify::LoadPlaylistResponse)),
          SLOT(StarredLoaded(pb::spotify::LoadPlaylistResponse)));
  connect(server_,
          SIGNAL(UserPlaylistLoaded(pb::spotify::LoadPlaylistResponse)),
          SLOT(UserPlaylistLoaded(pb::spotify::LoadPlaylistResponse)));
  connect(server_, SIGNAL(PlaybackError(QString)),
          SIGNAL(StreamError(QString)));
  connect(server_, SIGNAL(SearchResults(pb::spotify::SearchResponse)),
          SLOT(SearchResults(pb::spotify::SearchResponse)));
  connect(server_, SIGNAL(ImageLoaded(QString, QImage)),
          SIGNAL(ImageLoaded(QString, QImage)));
  connect(server_,
          SIGNAL(SyncPlaylistProgress(pb::spotify::SyncPlaylistProgress)),
          SLOT(SyncPlaylistProgress(pb::spotify::SyncPlaylistProgress)));
  connect(server_,
          SIGNAL(ToplistBrowseResults(pb::spotify::BrowseToplistResponse)),
          SLOT(ToplistLoaded(pb::spotify::BrowseToplistResponse)));

  server_->Init();

  login_task_id_ = app_->task_manager()->StartTask(tr("Connecting to Spotify"));

  QString login_username = username;
  QString login_password = password;

  if (username.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);

    login_username = s.value("username").toString();
    login_password = QString();
  }

  server_->Login(login_username, login_password, bitrate_,
                 volume_normalisation_);

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
      app_->task_manager()->SetTaskFinished(login_task_id_);
    }

#ifdef HAVE_SPOTIFY_DOWNLOADER
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

  connect(blob_process_, SIGNAL(error(QProcess::ProcessError)),
          SLOT(BlobProcessError(QProcess::ProcessError)));

  qLog(Info) << "Starting" << blob_path;
  blob_process_->start(
      blob_path, QStringList() << QString::number(server_->server_port()));
}

bool SpotifyService::IsBlobInstalled() const {
  return QFile::exists(system_blob_path_) || QFile::exists(local_blob_path_);
}

void SpotifyService::InstallBlob() {
#ifdef HAVE_SPOTIFY_DOWNLOADER
  // The downloader deletes itself when it finishes
  SpotifyBlobDownloader* downloader = new SpotifyBlobDownloader(
      local_blob_version_, QFileInfo(local_blob_path_).path(), this);
  connect(downloader, SIGNAL(Finished()), SLOT(BlobDownloadFinished()));
  connect(downloader, SIGNAL(Finished()), SIGNAL(BlobStateChanged()));
  downloader->Start();
#endif  // HAVE_SPOTIFY_DOWNLOADER
}

void SpotifyService::BlobDownloadFinished() { EnsureServerCreated(); }

void SpotifyService::AddCurrentSongToPlaylist(QAction* action) {
  int playlist_index = action->data().toInt();
  AddSongsToPlaylist(playlist_index, QList<QUrl>() << current_song_url_);
}

void SpotifyService::AddSongsToPlaylist(int playlist_index,
                                        const QList<QUrl>& songs_urls) {
  EnsureServerCreated();
  server_->AddSongsToPlaylist(playlist_index, songs_urls);
}

void SpotifyService::PlaylistsUpdated(const pb::spotify::Playlists& response) {
  if (login_task_id_) {
    app_->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  // Create starred and inbox playlists if they're not here already
  if (!search_) {
    search_ =
        new QStandardItem(IconLoader::Load("edit-find"), tr("Search results"));
    search_->setToolTip(
        tr("Start typing something on the search box above to "
           "fill this search results list"));
    search_->setData(Type_SearchResults, InternetModel::Role_Type);
    search_->setData(InternetModel::PlayBehaviour_MultipleItems,
                     InternetModel::Role_PlayBehaviour);

    starred_ = new QStandardItem(QIcon(":/star-on.png"), tr("Starred"));
    starred_->setData(Type_StarredPlaylist, InternetModel::Role_Type);
    starred_->setData(true, InternetModel::Role_CanLazyLoad);
    starred_->setData(InternetModel::PlayBehaviour_MultipleItems,
                      InternetModel::Role_PlayBehaviour);

    inbox_ = new QStandardItem(IconLoader::Load("mail-message"), tr("Inbox"));
    inbox_->setData(Type_InboxPlaylist, InternetModel::Role_Type);
    inbox_->setData(true, InternetModel::Role_CanLazyLoad);
    inbox_->setData(InternetModel::PlayBehaviour_MultipleItems,
                    InternetModel::Role_PlayBehaviour);

    toplist_ = new QStandardItem(QIcon(), tr("Top tracks"));
    toplist_->setData(Type_Toplist, InternetModel::Role_Type);
    toplist_->setData(true, InternetModel::Role_CanLazyLoad);
    toplist_->setData(InternetModel::PlayBehaviour_MultipleItems,
                      InternetModel::Role_PlayBehaviour);

    root_->appendRow(search_);
    root_->appendRow(toplist_);
    root_->appendRow(starred_);
    root_->appendRow(inbox_);
  }

  // Don't do anything if the playlists haven't changed since last time.
  if (!DoPlaylistsDiffer(response)) {
    qLog(Debug) << "Playlists haven't changed - not updating";
    return;
  }
  qLog(Debug) << "Playlist have changed: updating";

  // Remove and recreate the other playlists
  for (QStandardItem* item : playlists_) {
    item->parent()->removeRow(item->row());
  }
  playlists_.clear();

  for (int i = 0; i < response.playlist_size(); ++i) {
    const pb::spotify::Playlists::Playlist& msg = response.playlist(i);

    QString playlist_title = QStringFromStdString(msg.name());
    if (!msg.is_mine()) {
      const std::string& owner = msg.owner();
      playlist_title +=
          tr(", by ") + QString::fromUtf8(owner.c_str(), owner.size());
    }
    QStandardItem* item = new QStandardItem(playlist_title);
    item->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    item->setData(msg.index(), Role_UserPlaylistIndex);
    item->setData(msg.is_mine(), InternetModel::Role_CanBeModified);
    item->setData(InternetModel::PlayBehaviour_MultipleItems,
                  InternetModel::Role_PlayBehaviour);

    root_->appendRow(item);
    playlists_ << item;

    // Preload the playlist items so that drag & drop works immediately.
    LazyPopulate(item);
  }
}

bool SpotifyService::DoPlaylistsDiffer(
    const pb::spotify::Playlists& response) const {
  if (playlists_.count() != response.playlist_size()) {
    return true;
  }

  for (int i = 0; i < response.playlist_size(); ++i) {
    const pb::spotify::Playlists::Playlist& msg = response.playlist(i);
    const QStandardItem* item = PlaylistBySpotifyIndex(msg.index());

    if (!item) {
      return true;
    }

    if (QStringFromStdString(msg.name()) != item->text()) {
      return true;
    }

    if (msg.nb_tracks() != item->rowCount()) {
      return true;
    }
  }

  return false;
}

void SpotifyService::InboxLoaded(
    const pb::spotify::LoadPlaylistResponse& response) {
  if (inbox_) {
    FillPlaylist(inbox_, response);
  }
}

void SpotifyService::StarredLoaded(
    const pb::spotify::LoadPlaylistResponse& response) {
  if (starred_) {
    FillPlaylist(starred_, response);
  }
}

void SpotifyService::ToplistLoaded(
    const pb::spotify::BrowseToplistResponse& response) {
  if (toplist_) {
    FillPlaylist(toplist_, response.track());
  }
}

QStandardItem* SpotifyService::PlaylistBySpotifyIndex(int index) const {
  for (QStandardItem* item : playlists_) {
    if (item->data(Role_UserPlaylistIndex).toInt() == index) {
      return item;
    }
  }
  return nullptr;
}

void SpotifyService::UserPlaylistLoaded(
    const pb::spotify::LoadPlaylistResponse& response) {
  // Find a playlist with this index
  QStandardItem* item =
      PlaylistBySpotifyIndex(response.request().user_playlist_index());
  if (item) {
    FillPlaylist(item, response);
  }
}

void SpotifyService::FillPlaylist(
    QStandardItem* item,
    const google::protobuf::RepeatedPtrField<pb::spotify::Track>& tracks) {
  if (item->hasChildren()) item->removeRows(0, item->rowCount());

  for (int i = 0; i < tracks.size(); ++i) {
    Song song;
    SongFromProtobuf(tracks.Get(i), &song);

    QStandardItem* child = CreateSongItem(song);

    item->appendRow(child);
  }
}

void SpotifyService::FillPlaylist(
    QStandardItem* item, const pb::spotify::LoadPlaylistResponse& response) {
  qLog(Debug) << "Filling playlist:" << item->text();
  FillPlaylist(item, response.track());
}

void SpotifyService::SongFromProtobuf(const pb::spotify::Track& track,
                                      Song* song) {
  song->set_rating(track.starred() ? 1.0 : 0.0);
  song->set_title(QStringFromStdString(track.title()));
  song->set_album(QStringFromStdString(track.album()));
  song->set_length_nanosec(track.duration_msec() * kNsecPerMsec);
  song->set_score(track.popularity());
  song->set_disc(track.disc());
  song->set_track(track.track());
  song->set_year(track.year());
  song->set_url(QUrl(QStringFromStdString(track.uri())));
  song->set_art_automatic("spotify://image/" +
                          QStringFromStdString(track.album_art_id()));

  QStringList artists;
  for (int i = 0; i < track.artist_size(); ++i) {
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

QList<QAction*> SpotifyService::playlistitem_actions(const Song& song) {
  // Clear previous actions
  while (!playlistitem_actions_.isEmpty()) {
    QAction* action = playlistitem_actions_.takeFirst();
    delete action->menu();
    delete action;
  }

  // Create a menu with 'add to playlist' actions for each Spotify playlist
  QAction* add_to_playlists = new QAction(IconLoader::Load("list-add"),
                                          tr("Add to Spotify playlists"), this);
  QMenu* playlists_menu = new QMenu();
  for (const QStandardItem* playlist_item : playlists_) {
    if (!playlist_item->data(InternetModel::Role_CanBeModified).toBool()) {
      continue;
    }
    QAction* add_to_playlist = new QAction(playlist_item->text(), this);
    add_to_playlist->setData(playlist_item->data(Role_UserPlaylistIndex));
    playlists_menu->addAction(add_to_playlist);
  }
  connect(playlists_menu, SIGNAL(triggered(QAction*)),
          SLOT(AddCurrentSongToPlaylist(QAction*)));
  add_to_playlists->setMenu(playlists_menu);
  playlistitem_actions_.append(add_to_playlists);

  // Keep in mind the current song URL
  current_song_url_ = song.url();

  return playlistitem_actions_;
}

PlaylistItem::Options SpotifyService::playlistitem_options() const {
  return PlaylistItem::SeekDisabled;
}

QWidget* SpotifyService::HeaderWidget() const {
  if (IsLoggedIn()) return search_box_;
  return nullptr;
}

void SpotifyService::EnsureMenuCreated() {
  if (context_menu_) return;

  context_menu_ = new QMenu;
  context_menu_->addAction(GetNewShowConfigAction());

  playlist_context_menu_ = new QMenu;
  playlist_context_menu_->addActions(GetPlaylistActions());
  playlist_context_menu_->addSeparator();
  playlist_sync_action_ = playlist_context_menu_->addAction(
      IconLoader::Load("view-refresh"), tr("Make playlist available offline"),
      this, SLOT(SyncPlaylist()));
  playlist_context_menu_->addSeparator();
  playlist_context_menu_->addAction(GetNewShowConfigAction());

  song_context_menu_ = new QMenu;
  remove_from_playlist_ = song_context_menu_->addAction(
      IconLoader::Load("list-remove"), tr("Remove from playlist"), this,
      SLOT(RemoveCurrentFromPlaylist()));

  song_context_menu_->addAction(GetNewShowConfigAction());
}

void SpotifyService::ClearSearchResults() {
  if (search_) search_->removeRows(0, search_->rowCount());
}

void SpotifyService::SyncPlaylist() {
  QStandardItem* item = playlist_sync_action_->data().value<QStandardItem*>();
  Q_ASSERT(item);

  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_UserPlaylist: {
      int index = item->data(Role_UserPlaylistIndex).toInt();
      server_->SyncUserPlaylist(index);
      playlist_sync_ids_[index] =
          app_->task_manager()->StartTask(tr("Syncing Spotify playlist"));
      break;
    }
    case Type_InboxPlaylist:
      server_->SyncInbox();
      inbox_sync_id_ =
          app_->task_manager()->StartTask(tr("Syncing Spotify inbox"));
      break;
    case Type_StarredPlaylist:
      server_->SyncStarred();
      starred_sync_id_ =
          app_->task_manager()->StartTask(tr("Syncing Spotify starred tracks"));
      break;
    default:
      break;
  }
}

void SpotifyService::Search(const QString& text, bool now) {
  EnsureServerCreated();

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

void SpotifyService::DoSearch() {
  if (!pending_search_.isEmpty()) {
    server_->Search(pending_search_, 200);
  }
}

void SpotifyService::SearchResults(
    const pb::spotify::SearchResponse& response) {
  if (QStringFromStdString(response.request().query()) != pending_search_) {
    qLog(Debug) << "Old search result for"
                << QStringFromStdString(response.request().query())
                << "expecting" << pending_search_;
    return;
  }
  pending_search_.clear();

  SongList songs;
  for (int i = 0; i < response.result_size(); ++i) {
    Song song;
    SongFromProtobuf(response.result(i), &song);
    songs << song;
  }

  qLog(Debug) << "Got" << songs.count() << "results";

  ClearSearchResults();

  // Fill results list
  for (const Song& song : songs) {
    QStandardItem* child = CreateSongItem(song);
    search_->appendRow(child);
  }

  const QString did_you_mean_suggestion =
      QStringFromStdString(response.did_you_mean());
  qLog(Debug) << "Did you mean suggestion: " << did_you_mean_suggestion;
  if (!did_you_mean_suggestion.isEmpty()) {
    search_box_->did_you_mean()->Show(did_you_mean_suggestion);
  } else {
    // In case something else was previously displayed
    search_box_->did_you_mean()->hide();
  }

  QModelIndex index = model()->merged_model()->mapFromSource(search_->index());
  ScrollToIndex(index);
}

SpotifyServer* SpotifyService::server() const {
  SpotifyService* nonconst_this = const_cast<SpotifyService*>(this);

  if (QThread::currentThread() != thread()) {
    metaObject()->invokeMethod(nonconst_this, "EnsureServerCreated",
                               Qt::BlockingQueuedConnection);
  } else {
    nonconst_this->EnsureServerCreated();
  }

  return server_;
}

void SpotifyService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();
  QStandardItem* item = model()->itemFromIndex(model()->current_index());
  if (item) {
    int type = item->data(InternetModel::Role_Type).toInt();
    if (type == Type_InboxPlaylist || type == Type_StarredPlaylist ||
        type == InternetModel::Type_UserPlaylist) {
      playlist_sync_action_->setData(qVariantFromValue(item));
      playlist_context_menu_->popup(global_pos);
      return;
    } else if (type == InternetModel::Type_Track) {
      // Is this track contained in a playlist we can modify?
      bool is_playlist_modifiable =
          item->parent() &&
          item->parent()->data(InternetModel::Role_CanBeModified).toBool();
      remove_from_playlist_->setVisible(is_playlist_modifiable);

      song_context_menu_->popup(global_pos);
      return;
    }
  }

  context_menu_->popup(global_pos);
}

void SpotifyService::ItemDoubleClicked(QStandardItem* item) {}

void SpotifyService::DropMimeData(const QMimeData* data,
                                  const QModelIndex& index) {
  QVariant q_playlist_index = index.data(Role_UserPlaylistIndex);
  if (!q_playlist_index.isValid()) {
    // In case song was dropped on a playlist item, not on the playlist title/root element
    q_playlist_index = index.parent().data(Role_UserPlaylistIndex);
  }
  if (!q_playlist_index.isValid())
    return;

  AddSongsToPlaylist(q_playlist_index.toInt(), data->urls());
}

void SpotifyService::LoadImage(const QString& id) {
  EnsureServerCreated();
  server_->LoadImage(id);
}

void SpotifyService::SetPaused(bool paused) {
  EnsureServerCreated();
  server_->SetPaused(paused);
}

void SpotifyService::SyncPlaylistProgress(
    const pb::spotify::SyncPlaylistProgress& progress) {
  qLog(Debug) << "Sync progress:" << progress.sync_progress();
  int task_id = -1;
  switch (progress.request().type()) {
    case pb::spotify::Inbox:
      task_id = inbox_sync_id_;
      break;
    case pb::spotify::Starred:
      task_id = starred_sync_id_;
      break;
    case pb::spotify::UserPlaylist: {
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
  app_->task_manager()->SetTaskProgress(task_id, progress.sync_progress(), 100);
  if (progress.sync_progress() == 100) {
    app_->task_manager()->SetTaskFinished(task_id);
    if (progress.request().type() == pb::spotify::UserPlaylist) {
      playlist_sync_ids_.remove(task_id);
    }
  }
}

QAction* SpotifyService::GetNewShowConfigAction() {
  QAction* action = new QAction(IconLoader::Load("configure"),
                                tr("Configure Spotify..."), this);
  connect(action, SIGNAL(triggered()), this, SLOT(ShowConfig()));
  return action;
}

void SpotifyService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Spotify);
}

void SpotifyService::RemoveCurrentFromPlaylist() {
  const QModelIndexList& indexes(model()->selected_indexes());
  QMap<int, QList<int>> playlists_songs_indices;
  for (const QModelIndex& index : indexes) {
    if (index.parent().data(InternetModel::Role_Type).toInt() !=
        InternetModel::Type_UserPlaylist) {
      continue;
    }

    if (index.data(InternetModel::Role_Type).toInt() !=
        InternetModel::Type_Track) {
      continue;
    }

    int playlist_index = index.parent().data(Role_UserPlaylistIndex).toInt();
    int song_index = index.row();
    playlists_songs_indices[playlist_index] << song_index;
  }

  for (QMap<int, QList<int>>::const_iterator it =
           playlists_songs_indices.constBegin();
       it != playlists_songs_indices.constEnd(); ++it) {
    RemoveSongsFromPlaylist(it.key(), it.value());
  }
}

void SpotifyService::RemoveSongsFromPlaylist(
    int playlist_index, const QList<int>& songs_indices_to_remove) {
  server_->RemoveSongsFromPlaylist(playlist_index, songs_indices_to_remove);
}

void SpotifyService::Logout() {
  delete server_;
  delete blob_process_;
  server_ = nullptr;
  blob_process_ = nullptr;

  login_state_ = LoginState_OtherError;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("login_state", login_state_);
}
