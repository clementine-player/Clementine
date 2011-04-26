#include "radiomodel.h"
#include "spotifyserver.h"
#include "spotifyservice.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QProcess>
#include <QSettings>

const char* SpotifyService::kServiceName = "Spotify";
const char* SpotifyService::kSettingsGroup = "Spotify";

SpotifyService::SpotifyService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      server_(NULL),
      blob_process_(NULL),
      root_(NULL),
      starred_(NULL),
      inbox_(NULL),
      login_task_id_(0) {
  blob_path_ = QCoreApplication::applicationFilePath() + "-spotifyblob";
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
  if (!starred_) {
    starred_ = new QStandardItem(QIcon(":/star-on.png"), tr("Starred"));
    starred_->setData(Type_StarredPlaylist, RadioModel::Role_Type);
    starred_->setData(true, RadioModel::Role_CanLazyLoad);

    inbox_ = new QStandardItem(IconLoader::Load("mail-message"), tr("Inbox"));
    inbox_->setData(Type_InboxPlaylist, RadioModel::Role_Type);
    inbox_->setData(true, RadioModel::Role_CanLazyLoad);

    root_->appendRow(starred_);
    root_->appendRow(inbox_);
  }

  // Remove and recreate the other playlists
  qDeleteAll(playlists_);
  playlists_.clear();

  for (int i=0 ; i<response.playlist_size() ; ++i) {
    const protobuf::Playlists::Playlist& msg = response.playlist(i);

    QStandardItem* item = new QStandardItem(QStringFromStdString(msg.name()));
    item->setData(Type_Playlist, RadioModel::Role_Type);
    item->setData(true, RadioModel::Role_CanLazyLoad);
    item->setData(msg.index(), Role_PlaylistIndex);

    root_->appendRow(item);
  }
}
