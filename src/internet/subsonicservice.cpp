#include "subsonicurlhandler.h"
#include "subsonicservice.h"
#include "internetmodel.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/utilities.h"
#include "ui/iconloader.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "core/mergedproxymodel.h"
#include "core/database.h"
#include "core/closure.h"
#include "core/taskmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QSslConfiguration>
#include <QXmlStreamReader>
#include <QSortFilterProxyModel>
#include <QMenu>

const char* SubsonicService::kServiceName = "Subsonic";
const char* SubsonicService::kSettingsGroup = "Subsonic";
const char* SubsonicService::kApiVersion = "1.7.0";
const char* SubsonicService::kApiClientName = "Clementine";

const char* SubsonicService::kSongsTable = "subsonic_songs";
const char* SubsonicService::kFtsTable = "subsonic_songs_fts";

SubsonicService::SubsonicService(Application* app, InternetModel *parent)
  : InternetService(kServiceName, app, parent, parent),
    network_(new QNetworkAccessManager(this)),
    url_handler_(new SubsonicUrlHandler(this, this)),
    scanner_(new SubsonicLibraryScanner(this, this)),
    load_database_task_id_(0),
    context_menu_(NULL),
    root_(NULL),
    library_backend_(NULL),
    library_model_(NULL),
    library_filter_(NULL),
    library_sort_model_(new QSortFilterProxyModel(this)),
    login_state_(LoginState_OtherError)
{
  app_->player()->RegisterUrlHandler(url_handler_);

  connect(scanner_, SIGNAL(ScanFinished()),
          SLOT(ReloadDatabaseFinished()));

  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(),
                         kSongsTable,
                         QString::null,
                         QString::null,
                         kFtsTable);

  library_model_ = new LibraryModel(library_backend_, app_, this);
  library_model_->set_show_various_artists(false);
  library_model_->set_show_smart_playlists(false);

  library_filter_ = new LibraryFilterWidget(0);
  library_filter_->SetSettingsGroup(kSettingsGroup);
  library_filter_->SetLibraryModel(library_model_);
  library_filter_->SetFilterHint(tr("Search Subsonic"));
  library_filter_->SetAgeFilterEnabled(false);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  connect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(UpdateTotalSongCount(int)));

  connect(this, SIGNAL(LoginStateChanged(SubsonicService::LoginState)),
          SLOT(onLoginStateChanged(SubsonicService::LoginState)));

  context_menu_ = new QMenu;
  context_menu_->addActions(GetPlaylistActions());
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Refresh catalogue"), this, SLOT(ReloadDatabase()));
  context_menu_->addSeparator();
  context_menu_->addMenu(library_filter_->menu());
}

SubsonicService::~SubsonicService()
{
}

QStandardItem* SubsonicService::CreateRootItem()
{
  root_ = new QStandardItem(QIcon(":providers/subsonic.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SubsonicService::LazyPopulate(QStandardItem *item)
{
  switch (item->data(InternetModel::Role_Type).toInt())
  {
  case InternetModel::Type_Service:
    library_model_->Init();
    model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
    break;

  default:
    break;
  }
}

void SubsonicService::ShowContextMenu(const QPoint &global_pos)
{
  const bool is_valid = model()->current_index().model() == library_sort_model_;

  GetAppendToPlaylistAction()->setEnabled(is_valid);
  GetReplacePlaylistAction()->setEnabled(is_valid);
  GetOpenInNewPlaylistAction()->setEnabled(is_valid);
  context_menu_->popup(global_pos);
}

QWidget* SubsonicService::HeaderWidget() const
{
  return library_filter_;
}

void SubsonicService::ReloadSettings()
{
  QSettings s;
  s.beginGroup(kSettingsGroup);

  server_ = s.value("server").toString();
  username_ = s.value("username").toString();
  password_ = s.value("password").toString();

  Login();
}

void SubsonicService::Login()
{
  // Forget session ID
  network_->setCookieJar(new QNetworkCookieJar(network_));
  // Forget login state whilst waiting
  login_state_ = LoginState_Unknown;
  // Ping is enough to check credentials
  Ping();
}

void SubsonicService::Login(const QString &server, const QString &username, const QString &password)
{
  server_ = QString(server);
  username_ = QString(username);
  password_ = QString(password);
  Login();
}

void SubsonicService::Ping()
{
  QNetworkReply* reply = Send(BuildRequestUrl("ping"));
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(onPingFinished(QNetworkReply*)),
             reply);
}

QUrl SubsonicService::BuildRequestUrl(const QString &view)
{
  QUrl url(server_ + "rest/" + view + ".view");
  url.addQueryItem("v", kApiVersion);
  url.addQueryItem("c", kApiClientName);
  url.addQueryItem("u", username_);
  url.addQueryItem("p", password_);
  return url;
}

QNetworkReply* SubsonicService::Send(const QUrl &url)
{
  QNetworkRequest request(url);
  // Don't try and check the authenticity of the SSL certificate - it'll almost
  // certainly be self-signed.
  QSslConfiguration sslconfig = QSslConfiguration::defaultConfiguration();
  sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  request.setSslConfiguration(sslconfig);
  QNetworkReply *reply = network_->get(request);
  return reply;
}

void SubsonicService::UpdateTotalSongCount(int count)
{
  if (count == 0 && !load_database_task_id_)
    ReloadDatabase();
}

void SubsonicService::ReloadDatabase()
{
  if (!load_database_task_id_)
    load_database_task_id_ = app_->task_manager()->StartTask(tr("Fetching Subsonic library"));
  scanner_->Scan();
}

void SubsonicService::ReloadDatabaseFinished()
{
  app_->task_manager()->SetTaskFinished(load_database_task_id_);
  load_database_task_id_ = 0;

  library_backend_->DeleteAll();
  library_backend_->AddOrUpdateSongs(scanner_->GetSongs());
}

void SubsonicService::onLoginStateChanged(SubsonicService::LoginState newstate)
{
  // TODO: library refresh logic?
}

void SubsonicService::onPingFinished(QNetworkReply *reply)
{
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError)
  {
    login_state_ = LoginState_BadServer;
    qLog(Error) << "Failed to connect ("
                << Utilities::EnumToString(QNetworkReply::staticMetaObject, "NetworkError", reply->error())
                << "):" << reply->errorString();
  }
  else
  {
    QXmlStreamReader reader(reply);
    reader.readNextStartElement();
    QStringRef status = reader.attributes().value("status");
    if (status == "ok")
    {
      login_state_ = LoginState_Loggedin;
    }
    else
    {
      reader.readNextStartElement();
      int error = reader.attributes().value("code").toString().toInt();
      qLog(Error) << "Subsonic error ("
                  << Utilities::EnumToString(SubsonicService::staticMetaObject, "ApiError", error)
                  << "):" << reader.attributes().value("message").toString();
      switch (error)
      {
      // "Parameter missing" for "ping" is always blank username or password
      case ApiError_ParameterMissing:
      case ApiError_BadCredentials:
        login_state_ = LoginState_BadCredentials;
        break;
      case ApiError_OutdatedClient:
        login_state_ = LoginState_OutdatedClient;
        break;
      case ApiError_OutdatedServer:
        login_state_ = LoginState_OutdatedServer;
        break;
      case ApiError_Unlicensed:
        login_state_ = LoginState_Unlicensed;
        break;
      default:
        login_state_ = LoginState_OtherError;
        break;
      }
    }
  }
  qLog(Debug) << "Login state changed:"
              << Utilities::EnumToString(SubsonicService::staticMetaObject, "LoginState", login_state_);
  emit LoginStateChanged(login_state_);
}


const int SubsonicLibraryScanner::kAlbumChunkSize = 500;
const int SubsonicLibraryScanner::kConcurrentRequests = 8;

SubsonicLibraryScanner::SubsonicLibraryScanner(SubsonicService* service, QObject* parent)
  : QObject(parent),
    service_(service),
    scanning_(false)
{
}

SubsonicLibraryScanner::~SubsonicLibraryScanner()
{
}

void SubsonicLibraryScanner::Scan()
{
  if (scanning_)
    return;

  album_queue_.clear();
  pending_requests_.clear();
  songs_.clear();
  scanning_ = true;
  GetAlbumList(0);
}

void SubsonicLibraryScanner::onGetAlbumListFinished(QNetworkReply *reply, int offset)
{
  reply->deleteLater();

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "subsonic-response");
  if (reader.attributes().value("status") != "ok") {
    // TODO: error handling
    return;
  }

  int albums_added = 0;
  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "albumList2");
  while (reader.readNextStartElement()) {
    Q_ASSERT(reader.name() == "album");
    album_queue_ << reader.attributes().value("id").toString();
    albums_added++;
    reader.skipCurrentElement();
  }

  if (albums_added > 0) {
    // Non-empty reply means potentially more albums to fetch
    GetAlbumList(offset + kAlbumChunkSize);
  } else if (album_queue_.size() == 0) {
    // Empty reply and no albums means an empty Subsonic server
    scanning_ = false;
  } else {
    // Empty reply but we have some albums, time to start fetching songs
    // Start up the maximum number of concurrent requests, finished requests get replaced with new ones
    for (int i = 0; i < kConcurrentRequests && !album_queue_.empty(); ++i) {
      GetAlbum(album_queue_.dequeue());
    }
  }
}

void SubsonicLibraryScanner::onGetAlbumFinished(QNetworkReply *reply)
{
  reply->deleteLater();
  pending_requests_.remove(reply);

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "subsonic-response");
  if (reader.attributes().value("status") != "ok") {
    // TODO: error handling
    return;
  }

  // Read album information
  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "album");
  QString album_artist = reader.attributes().value("artist").toString();

  // Read song information
  while (reader.readNextStartElement()) {
    Q_ASSERT(reader.name() == "song");
    Song song;
    QString id = reader.attributes().value("id").toString();
    song.set_title(reader.attributes().value("title").toString());
    song.set_album(reader.attributes().value("album").toString());
    song.set_track(reader.attributes().value("track").toString().toInt());
    song.set_artist(reader.attributes().value("artist").toString());
    song.set_albumartist(album_artist);
    song.set_bitrate(reader.attributes().value("bitRate").toString().toInt());
    song.set_year(reader.attributes().value("year").toString().toInt());
    song.set_genre(reader.attributes().value("genre").toString());
    qint64 length = reader.attributes().value("duration").toString().toInt();
    length *= 1000000000;
    song.set_length_nanosec(length);
    QUrl url = QUrl(QString("subsonic://%1").arg(id));
    song.set_url(url);
    song.set_filesize(reader.attributes().value("size").toString().toInt());
    // We need to set these to satisfy the database constraints
    song.set_directory_id(0);
    song.set_mtime(0);
    song.set_ctime(0);
    songs_ << song;
    reader.skipCurrentElement();
  }

  // Start the next request if albums remain
  if (!album_queue_.empty())
    GetAlbum(album_queue_.dequeue());

  // If this was the last response, we're done!
  if (album_queue_.empty() && pending_requests_.empty()) {
    scanning_ = false;
    emit ScanFinished();
  }
}

void SubsonicLibraryScanner::GetAlbumList(int offset)
{
  QUrl url = service_->BuildRequestUrl("getAlbumList2");
  url.addQueryItem("type", "alphabeticalByName");
  url.addQueryItem("size", QString::number(kAlbumChunkSize));
  url.addQueryItem("offset", QString::number(offset));
  QNetworkReply* reply = service_->Send(url);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(onGetAlbumListFinished(QNetworkReply*,int)),
             reply, offset);
}

void SubsonicLibraryScanner::GetAlbum(QString id)
{
  QUrl url = service_->BuildRequestUrl("getAlbum");
  url.addQueryItem("id", id);
  QNetworkReply* reply = service_->Send(url);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(onGetAlbumFinished(QNetworkReply*)),
             reply);
  pending_requests_.insert(reply);
}
