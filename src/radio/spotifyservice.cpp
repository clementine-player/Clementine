#include "radiomodel.h"
#include "spotifyserver.h"
#include "spotifyservice.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/taskmanager.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "library/librarymodel.h"
#include "spotifyblob/spotifymessagehandler.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QMenu>
#include <QProcess>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTcpServer>
#include <QTemporaryFile>

const char* SpotifyService::kServiceName = "Spotify";
const char* SpotifyService::kSettingsGroup = "Spotify";
const char* SpotifyService::kSearchSongsTable = "spotify_search_songs";
const char* SpotifyService::kSearchFtsTable = "spotify_search_songs_fts";

SpotifyService::SpotifyService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      server_(NULL),
      blob_process_(NULL),
      root_(NULL),
      search_results_(NULL),
      starred_(NULL),
      inbox_(NULL),
      login_task_id_(0),
      context_menu_(NULL),
      library_filter_(NULL),
      library_sort_model_(new QSortFilterProxyModel(this)) {
#ifdef Q_OS_DARWIN
  blob_path_ = QCoreApplication::applicationDirPath() + "/../Resources/clementine-spotifyblob";
#else
  blob_path_ = QCoreApplication::applicationFilePath() + "-spotifyblob";
#endif
  qLog(Debug) << "Loading spotify blob from:" << blob_path_;

  // Create the library backend in the database thread
  library_backend_ = new LibraryBackend;
  library_backend_->Init(parent->db_thread()->Worker(),
                         kSearchSongsTable, QString(), QString(), kSearchFtsTable);
  library_model_ = new LibraryModel(library_backend_, parent->task_manager(), this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);
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
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
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
}

void SpotifyService::EnsureServerCreated(const QString& username,
                                         const QString& password) {
  if (server_) {
    return;
  }

  qLog(Debug) << Q_FUNC_INFO;

  server_ = new SpotifyServer(this);
  blob_process_ = new QProcess(this);
  blob_process_->setProcessChannelMode(QProcess::ForwardedChannels);

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

  connect(blob_process_,
          SIGNAL(error(QProcess::ProcessError)),
          SLOT(BlobProcessError(QProcess::ProcessError)));

  server_->Init();
  blob_process_->start(
        blob_path_, QStringList() << QString::number(server_->server_port()));

  login_task_id_ = model()->task_manager()->StartTask(tr("Connecting to Spotify"));

  if (username.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);

    server_->Login(s.value("username").toString(), s.value("password").toString());
  } else {
    server_->Login(username, password);
  }
}

void SpotifyService::PlaylistsUpdated(const protobuf::Playlists& response) {
  if (login_task_id_) {
    model()->task_manager()->SetTaskFinished(login_task_id_);
    login_task_id_ = 0;
  }

  // Create starred and inbox playlists if they're not here already
  if (!search_results_) {
    search_results_ = new QStandardItem(tr("Search results"));
    search_results_->setData(Type_SearchResults, RadioModel::Role_Type);
    search_results_->setData(true, RadioModel::Role_CanLazyLoad);

    starred_ = new QStandardItem(QIcon(":/star-on.png"), tr("Starred"));
    starred_->setData(Type_StarredPlaylist, RadioModel::Role_Type);
    starred_->setData(true, RadioModel::Role_CanLazyLoad);

    inbox_ = new QStandardItem(IconLoader::Load("mail-message"), tr("Inbox"));
    inbox_->setData(Type_InboxPlaylist, RadioModel::Role_Type);
    inbox_->setData(true, RadioModel::Role_CanLazyLoad);

    root_->appendRow(search_results_);
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
    child->setData(QUrl(song.filename()), RadioModel::Role_Url);

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
  song->set_filename(QStringFromStdString(track.uri()));

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
  return PlaylistItem::SpecialPlayBehaviour |
         PlaylistItem::PauseDisabled;
}

PlaylistItem::SpecialLoadResult SpotifyService::StartLoading(const QUrl& url) {
  // Pick an unused local port.  There's a possible race condition here -
  // something else might grab the port before gstreamer does.
  quint16 port = 0;

  {
    QTcpServer server;
    server.listen(QHostAddress::LocalHost);
    port = server.serverPort();
  }

  if (port == 0) {
    qLog(Warning) << "Couldn't pick an unused port";
    return PlaylistItem::SpecialLoadResult();
  }

  // Tell Spotify to start sending to this port
  EnsureServerCreated();
  server_->StartPlayback(url.toString(), port);

  // Tell gstreamer to listen on this port
  return PlaylistItem::SpecialLoadResult(
        PlaylistItem::SpecialLoadResult::TrackAvailable,
        url,
        QUrl("tcp://localhost:" + QString::number(port)));
}

void SpotifyService::EnsureMenuCreated() {
  if (context_menu_)
    return;

  context_menu_ = new QMenu;

  context_menu_->addActions(GetPlaylistActions());
  context_menu_->addSeparator();
  QAction* config_action = context_menu_->addAction(IconLoader::Load("configure"), tr("Configure Spotify..."), this, SLOT(ShowConfig()));

  library_filter_ = new LibraryFilterWidget(0);
  library_filter_->SetSettingsGroup(kSettingsGroup);
  library_filter_->SetLibraryModel(library_model_);
  library_filter_->SetFilterHint(tr("Search Spotify"));
  library_filter_->SetAgeFilterEnabled(false);
  library_filter_->SetGroupByEnabled(false);
  library_filter_->AddMenuAction(config_action);
  library_filter_->SetApplyFilterToLibrary(false);
  library_filter_->SetDelayBehaviour(LibraryFilterWidget::AlwaysDelayed);

  connect(library_filter_, SIGNAL(Filter(QString)), SLOT(Search(QString)));
}

QWidget* SpotifyService::HeaderWidget() const {
  const_cast<SpotifyService*>(this)->EnsureMenuCreated();
  return library_filter_;
}

void SpotifyService::Search(const QString& text) {
  if (!text.isEmpty()) {
    pending_search_ = text;
    server_->Search(text, 250);
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

  library_backend_->DeleteAll();
  library_backend_->AddOrUpdateSongs(songs);
}
