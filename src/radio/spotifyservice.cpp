#include "config.h"
#include "radiomodel.h"
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

const char* SpotifyService::kServiceName = "Spotify";
const char* SpotifyService::kSettingsGroup = "Spotify";
const char* SpotifyService::kBlobDownloadUrl = "http://spotify.clementine-player.org/";
const int SpotifyService::kSearchDelayMsec = 400;

SpotifyService::SpotifyService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      server_(NULL),
      url_handler_(new SpotifyUrlHandler(this, this)),
      blob_process_(NULL),
      root_(NULL),
      starred_(NULL),
      inbox_(NULL),
      login_task_id_(0),
      pending_search_playlist_(NULL),
      context_menu_(NULL),
      search_delay_(new QTimer(this)) {
  // Build the search path for the binary blob.
  // Look for one distributed alongside clementine first, then check in the
  // user's home directory for any that have been downloaded.
  blob_path_ << QCoreApplication::applicationFilePath() + "-spotifyblob" CMAKE_EXECUTABLE_SUFFIX
             << QCoreApplication::applicationDirPath() + "/../PlugIns/clementine-spotifyblob" CMAKE_EXECUTABLE_SUFFIX;

  local_blob_version_ = QString("version%1-%2bit").arg(SPOTIFY_BLOB_VERSION).arg(sizeof(void*) * 8);
  local_blob_path_    = Utilities::GetConfigPath(Utilities::Path_LocalSpotifyBlob) +
                        "/" + local_blob_version_ + "/blob";

  qLog(Debug) << "Spotify blob search path:" << blob_path_;
  qLog(Debug) << "Spotify local blob path:" << local_blob_path_;

  model()->player()->RegisterUrlHandler(url_handler_);
  model()->player()->playlists()->RegisterSpecialPlaylistType(
        new SpotifySearchPlaylistType(this));

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));
}

SpotifyService::~SpotifyService() {
}

QStandardItem* SpotifyService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":icons/svg/spotify.svg"), kServiceName);
  root_->setData(true, RadioModel::Role_CanLazyLoad);
  return root_;
}

void SpotifyService::LazyPopulate(QStandardItem* item) {
  switch (item->data(RadioModel::Role_Type).toInt()) {
    case RadioModel::Type_Service:
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
  delete server_;
  delete blob_process_;
  server_ = NULL;
  blob_process_ = NULL;

  EnsureServerCreated(username, password);
}

void SpotifyService::LoginCompleted(bool success) {
  if (login_task_id_) {
    model()->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  emit LoginFinished(success);
}

void SpotifyService::BlobProcessError(QProcess::ProcessError error) {
  qLog(Error) << "Spotify blob process failed:" << error;
  blob_process_->deleteLater();
  blob_process_ = NULL;
}

void SpotifyService::EnsureServerCreated(const QString& username,
                                         const QString& password) {
  if (server_ && blob_process_) {
    return;
  }

  delete server_;
  server_ = new SpotifyServer(this);

  connect(server_, SIGNAL(LoginCompleted(bool)), SLOT(LoginCompleted(bool)));
  connect(server_, SIGNAL(PlaylistsUpdated(protobuf::Playlists)),
          SLOT(PlaylistsUpdated(protobuf::Playlists)));
  connect(server_, SIGNAL(InboxLoaded(protobuf::LoadPlaylistResponse)),
          SLOT(InboxLoaded(protobuf::LoadPlaylistResponse)));
  connect(server_, SIGNAL(StarredLoaded(protobuf::LoadPlaylistResponse)),
          SLOT(StarredLoaded(protobuf::LoadPlaylistResponse)));
  connect(server_, SIGNAL(UserPlaylistLoaded(protobuf::LoadPlaylistResponse)),
          SLOT(UserPlaylistLoaded(protobuf::LoadPlaylistResponse)));
  connect(server_, SIGNAL(PlaybackError(QString)),
          SIGNAL(StreamError(QString)));
  connect(server_, SIGNAL(SearchResults(protobuf::SearchResponse)),
          SLOT(SearchResults(protobuf::SearchResponse)));
  connect(server_, SIGNAL(ImageLoaded(QString,QImage)),
          SLOT(ImageLoaded(QString,QImage)));

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
  foreach (const QString& path, blob_path_) {
    if (QFile::exists(path)) {
      blob_path = path;
      break;
    }
  }

  // Next look in the local path
  const QString local_blob_dir = QFileInfo(local_blob_path_).path();
  if (blob_path.isEmpty()) {
    if (QFile::exists(local_blob_path_)) {
      blob_path = local_blob_path_;
      env.insert("LD_LIBRARY_PATH", local_blob_dir);
    }
  }

  if (blob_path.isEmpty()) {
    // If the blob still wasn't found then we'll prompt the user to download one
    if (login_task_id_) {
      model()->task_manager()->SetTaskFinished(login_task_id_);
    }

    #ifdef Q_OS_LINUX
      QMessageBox::StandardButton ret = QMessageBox::question(NULL,
          tr("Spotify plugin not installed"),
          tr("An additional plugin is required to use Spotify in Clementine.  Would you like to download and install it now?"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

      if (ret == QMessageBox::Yes) {
        // The downloader deletes itself when it finishes
        SpotifyBlobDownloader* downloader = new SpotifyBlobDownloader(
              local_blob_version_, local_blob_dir, this);
        connect(downloader, SIGNAL(Finished()), SLOT(BlobDownloadFinished()));
        downloader->Start();
      }
    #endif

    return;
  }

  delete blob_process_;
  blob_process_ = new QProcess(this);
  blob_process_->setProcessChannelMode(QProcess::ForwardedChannels);

  connect(blob_process_,
          SIGNAL(error(QProcess::ProcessError)),
          SLOT(BlobProcessError(QProcess::ProcessError)));

  qLog(Info) << "Starting" << blob_path;
  blob_process_->start(
        blob_path, QStringList() << QString::number(server_->server_port()));
}

void SpotifyService::BlobDownloadFinished() {
  EnsureServerCreated();
}

void SpotifyService::PlaylistsUpdated(const protobuf::Playlists& response) {
  if (login_task_id_) {
    model()->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  // Create starred and inbox playlists if they're not here already
  if (!search_) {
    search_ = new QStandardItem(IconLoader::Load("edit-find"),
                                tr("Search Spotify (opens a new tab)"));
    search_->setData(Type_SearchResults, RadioModel::Role_Type);
    search_->setData(RadioModel::PlayBehaviour_DoubleClickAction,
                             RadioModel::Role_PlayBehaviour);

    starred_ = new QStandardItem(QIcon(":/star-on.png"), tr("Starred"));
    starred_->setData(Type_StarredPlaylist, RadioModel::Role_Type);
    starred_->setData(true, RadioModel::Role_CanLazyLoad);

    inbox_ = new QStandardItem(IconLoader::Load("mail-message"), tr("Inbox"));
    inbox_->setData(Type_InboxPlaylist, RadioModel::Role_Type);
    inbox_->setData(true, RadioModel::Role_CanLazyLoad);

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
    const protobuf::Playlists::Playlist& msg = response.playlist(i);

    QStandardItem* item = new QStandardItem(QStringFromStdString(msg.name()));
    item->setData(Type_UserPlaylist, RadioModel::Role_Type);
    item->setData(true, RadioModel::Role_CanLazyLoad);
    item->setData(msg.index(), Role_UserPlaylistIndex);

    root_->appendRow(item);
    playlists_ << item;
  }
}

bool SpotifyService::DoPlaylistsDiffer(const protobuf::Playlists& response) {
  if (playlists_.count() != response.playlist_size()) {
    return true;
  }

  for (int i=0 ; i<response.playlist_size() ; ++i) {
    const protobuf::Playlists::Playlist& msg = response.playlist(i);
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

void SpotifyService::InboxLoaded(const protobuf::LoadPlaylistResponse& response) {
  FillPlaylist(inbox_, response);
}

void SpotifyService::StarredLoaded(const protobuf::LoadPlaylistResponse& response) {
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

void SpotifyService::UserPlaylistLoaded(const protobuf::LoadPlaylistResponse& response) {
  // Find a playlist with this index
  QStandardItem* item = PlaylistBySpotifyIndex(response.request().user_playlist_index());
  if (item) {
    FillPlaylist(item, response);
  }
}

void SpotifyService::FillPlaylist(QStandardItem* item, const protobuf::LoadPlaylistResponse& response) {
  if (item->hasChildren())
    item->removeRows(0, item->rowCount());

  for (int i=0 ; i<response.track_size() ; ++i) {
    Song song;
    SongFromProtobuf(response.track(i), &song);

    QStandardItem* child = new QStandardItem(song.PrettyTitleWithArtist());
    child->setData(Type_Track, RadioModel::Role_Type);
    child->setData(QVariant::fromValue(song), RadioModel::Role_SongMetadata);
    child->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);
    child->setData(song.url(), RadioModel::Role_Url);

    item->appendRow(child);
  }
}

void SpotifyService::SongFromProtobuf(const protobuf::Track& track, Song* song) const {
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

void SpotifyService::SearchResults(const protobuf::SearchResponse& response) {
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

void SpotifyService::ShowConfig() {
  emit OpenSettingsAtPage(SettingsDialog::Page_Spotify);
}

