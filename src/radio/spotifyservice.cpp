#include "core/logging.h"
#include "radiomodel.h"
#include "spotifyserver.h"
#include "spotifyservice.h"

#include <QCoreApplication>
#include <QProcess>

const char* SpotifyService::kServiceName = "Spotify";
const char* SpotifyService::kSettingsGroup = "Spotify";

SpotifyService::SpotifyService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      server_(NULL),
      blob_process_(NULL) {
  blob_path_ = QCoreApplication::applicationFilePath() + "-spotifyblob";
}

SpotifyService::~SpotifyService() {
}

QStandardItem* SpotifyService::CreateRootItem() {
  QStandardItem* item = new QStandardItem(QIcon(":icons/svg/spotify.svg"), kServiceName);
  item->setData(true, RadioModel::Role_CanLazyLoad);
  return item;
}

void SpotifyService::LazyPopulate(QStandardItem* parent) {
  return;
}

QModelIndex SpotifyService::GetCurrentIndex() {
  return QModelIndex();
}

void SpotifyService::Login(const QString& username, const QString& password) {
  qLog(Debug) << Q_FUNC_INFO;
  delete server_;
  delete blob_process_;

  server_ = new SpotifyServer(this);
  blob_process_ = new QProcess(this);
  blob_process_->setProcessChannelMode(QProcess::ForwardedChannels);

  connect(server_, SIGNAL(ClientConnected()), SLOT(ClientConnected()));
  connect(server_, SIGNAL(LoginCompleted(bool)), SLOT(LoginCompleted(bool)));

  connect(blob_process_,
          SIGNAL(error(QProcess::ProcessError)),
          SLOT(BlobProcessError(QProcess::ProcessError)));

  pending_username_ = username;
  pending_password_ = password;

  server_->Init();
  blob_process_->start(
        blob_path_, QStringList() << QString::number(server_->server_port()));
}

void SpotifyService::ClientConnected() {
  qLog(Debug) << "ClientConnected";

  server_->Login(pending_username_, pending_password_);
}

void SpotifyService::LoginCompleted(bool success) {
  emit LoginFinished(success);
}

void SpotifyService::BlobProcessError(QProcess::ProcessError error) {
  qLog(Error) << "Failed to start blob process:" << error;
}
