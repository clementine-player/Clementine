#include "subsonicservice.h"

#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QSslConfiguration>
#include <QXmlStreamReader>

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/internetmodel.h"
#include "internet/subsonicurlhandler.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "ui/iconloader.h"

const char* SubsonicService::kServiceName = "Subsonic";
const char* SubsonicService::kSettingsGroup = "Subsonic";
const char* SubsonicService::kApiVersion = "1.8.0";
const char* SubsonicService::kApiClientName = "Clementine";

const char* SubsonicService::kSongsTable = "subsonic_songs";
const char* SubsonicService::kFtsTable = "subsonic_songs_fts";

const int SubsonicService::kMaxRedirects = 10;

SubsonicService::SubsonicService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      network_(new QNetworkAccessManager(this)),
      url_handler_(new SubsonicUrlHandler(this, this)),
      scanner_(new SubsonicLibraryScanner(this, this)),
      load_database_task_id_(0),
      context_menu_(nullptr),
      root_(nullptr),
      library_backend_(nullptr),
      library_model_(nullptr),
      library_filter_(nullptr),
      library_sort_model_(new QSortFilterProxyModel(this)),
      total_song_count_(0),
      login_state_(LoginState_OtherError),
      redirect_count_(0) {
  app_->player()->RegisterUrlHandler(url_handler_);

  connect(scanner_, SIGNAL(ScanFinished()), SLOT(ReloadDatabaseFinished()));

  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(), kSongsTable, QString::null,
                         QString::null, kFtsTable);
  connect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(UpdateTotalSongCount(int)));

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
  library_sort_model_->setSortLocaleAware(true);
  library_sort_model_->sort(0);

  connect(this, SIGNAL(LoginStateChanged(SubsonicService::LoginState)),
          SLOT(OnLoginStateChanged(SubsonicService::LoginState)));

  context_menu_ = new QMenu;
  context_menu_->addActions(GetPlaylistActions());
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("view-refresh"),
                           tr("Refresh catalogue"), this,
                           SLOT(ReloadDatabase()));
  QAction* config_action = context_menu_->addAction(
      IconLoader::Load("configure"), tr("Configure Subsonic..."), this,
      SLOT(ShowConfig()));
  context_menu_->addSeparator();
  context_menu_->addMenu(library_filter_->menu());

  library_filter_->AddMenuAction(config_action);

  app_->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_, tr("Subsonic"), "subsonic",
      QIcon(":/providers/subsonic.png"), true, app_, this));
}

SubsonicService::~SubsonicService() {}

QStandardItem* SubsonicService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/subsonic.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SubsonicService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      library_model_->Init();
      if (login_state() != LoginState_Loggedin) {
        ShowConfig();
      } else if (total_song_count_ == 0 && !load_database_task_id_) {
        ReloadDatabase();
      }
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void SubsonicService::ShowContextMenu(const QPoint& global_pos) {
  const bool is_valid = model()->current_index().model() == library_sort_model_;

  GetAppendToPlaylistAction()->setEnabled(is_valid);
  GetReplacePlaylistAction()->setEnabled(is_valid);
  GetOpenInNewPlaylistAction()->setEnabled(is_valid);
  context_menu_->popup(global_pos);
}

QWidget* SubsonicService::HeaderWidget() const { return library_filter_; }

void SubsonicService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  UpdateServer(s.value("server").toString());
  username_ = s.value("username").toString();
  password_ = s.value("password").toString();
  usesslv3_ = s.value("usesslv3").toBool();

  Login();
}

bool SubsonicService::IsConfigured() const {
  return !configured_server_.isEmpty() && !username_.isEmpty() &&
         !password_.isEmpty();
}

void SubsonicService::Login() {
  // Recreate fresh network state, otherwise old HTTPS settings seem to get
  // reused
  network_->deleteLater();
  network_ = new QNetworkAccessManager(this);
  network_->setCookieJar(new QNetworkCookieJar(network_));
  // Forget login state whilst waiting
  login_state_ = LoginState_Unknown;

  if (IsConfigured()) {
    // Ping is enough to check credentials
    Ping();
  } else {
    login_state_ = LoginState_IncompleteCredentials;
    emit LoginStateChanged(login_state_);
  }
}

void SubsonicService::Login(const QString& server, const QString& username,
                            const QString& password, const bool& usesslv3) {
  UpdateServer(server);
  username_ = username;
  password_ = password;
  usesslv3_ = usesslv3;
  Login();
}

void SubsonicService::Ping() {
  QNetworkReply* reply = Send(BuildRequestUrl("ping"));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(OnPingFinished(QNetworkReply*)), reply);
}

QUrl SubsonicService::BuildRequestUrl(const QString& view) const {
  QUrl url(working_server_ + "/rest/" + view + ".view");
  url.addQueryItem("v", kApiVersion);
  url.addQueryItem("c", kApiClientName);
  url.addQueryItem("u", username_);
  url.addQueryItem("p", password_);
  return url;
}

QUrl SubsonicService::ScrubUrl(const QUrl& url) {
  QUrl return_url(url);
  QString path = url.path();
  int rest_location = path.lastIndexOf("/rest", -1, Qt::CaseInsensitive);
  if (rest_location >= 0) {
    return_url.setPath(path.left(rest_location));
  }
  return return_url;
}

QNetworkReply* SubsonicService::Send(const QUrl& url) {
  QNetworkRequest request(url);
  // Don't try and check the authenticity of the SSL certificate - it'll almost
  // certainly be self-signed.
  QSslConfiguration sslconfig = QSslConfiguration::defaultConfiguration();
  sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  if (usesslv3_) {
    sslconfig.setProtocol(QSsl::SslV3);
  }
  request.setSslConfiguration(sslconfig);
  QNetworkReply* reply = network_->get(request);
  return reply;
}

void SubsonicService::UpdateTotalSongCount(int count) {
  total_song_count_ = count;
}

void SubsonicService::ReloadDatabase() {
  if (!load_database_task_id_) {
    load_database_task_id_ =
        app_->task_manager()->StartTask(tr("Fetching Subsonic library"));
  }
  scanner_->Scan();
}

void SubsonicService::ReloadDatabaseFinished() {
  app_->task_manager()->SetTaskFinished(load_database_task_id_);
  load_database_task_id_ = 0;

  library_backend_->DeleteAll();
  library_backend_->AddOrUpdateSongs(scanner_->GetSongs());
  library_model_->Reset();
}

void SubsonicService::OnLoginStateChanged(
    SubsonicService::LoginState newstate) {
  // TODO: library refresh logic?
}

void SubsonicService::OnPingFinished(QNetworkReply* reply) {
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    switch (reply->error()) {
      case QNetworkReply::ConnectionRefusedError:
        login_state_ = LoginState_ConnectionRefused;
        break;
      case QNetworkReply::HostNotFoundError:
        login_state_ = LoginState_HostNotFound;
        break;
      case QNetworkReply::TimeoutError:
        login_state_ = LoginState_Timeout;
        break;
      case QNetworkReply::SslHandshakeFailedError:
        login_state_ = LoginState_SslError;
        break;
      default:  // Treat uncaught error types here as generic
        login_state_ = LoginState_BadServer;
        break;
    }
    qLog(Error) << "Failed to connect ("
                << Utilities::EnumToString(QNetworkReply::staticMetaObject,
                                           "NetworkError", reply->error())
                << "):" << reply->errorString();
  } else {
    QXmlStreamReader reader(reply);
    reader.readNextStartElement();
    QStringRef status = reader.attributes().value("status");
    int http_status_code =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == "ok") {
      login_state_ = LoginState_Loggedin;
    } else if (http_status_code >= 300 && http_status_code <= 399) {
      // Received a redirect status code, follow up on it.
      QUrl redirect_url =
          reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
      if (redirect_url.isEmpty()) {
        qLog(Debug) << "Received HTTP code " << http_status_code
                    << ", but no URL";
        login_state_ = LoginState_RedirectNoUrl;
      } else {
        redirect_count_++;
        qLog(Debug) << "Redirect receieved to "
                    << redirect_url.toString(QUrl::RemoveQuery)
                    << ", current redirect count is " << redirect_count_;
        if (redirect_count_ <= kMaxRedirects) {
          working_server_ = ScrubUrl(redirect_url).toString(QUrl::RemoveQuery);
          Ping();
          // To avoid the LoginStateChanged, as it will come from the recursive
          // request.
          return;
        } else {
          // Redirect limit exceeded
          login_state_ = LoginState_RedirectLimitExceeded;
        }
      }
    } else {
      reader.readNextStartElement();
      int error = reader.attributes().value("code").toString().toInt();
      qLog(Error) << "Subsonic error ("
                  << Utilities::EnumToString(SubsonicService::staticMetaObject,
                                             "ApiError", error)
                  << "):" << reader.attributes().value("message").toString();
      switch (error) {
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
              << Utilities::EnumToString(SubsonicService::staticMetaObject,
                                         "LoginState", login_state_);
  emit LoginStateChanged(login_state_);
}

void SubsonicService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Subsonic);
}

void SubsonicService::UpdateServer(const QString& server) {
  configured_server_ = server;
  working_server_ = server;
  redirect_count_ = 0;
}

const int SubsonicLibraryScanner::kAlbumChunkSize = 500;
const int SubsonicLibraryScanner::kConcurrentRequests = 8;

SubsonicLibraryScanner::SubsonicLibraryScanner(SubsonicService* service,
                                               QObject* parent)
    : QObject(parent), service_(service), scanning_(false) {}

SubsonicLibraryScanner::~SubsonicLibraryScanner() {}

void SubsonicLibraryScanner::Scan() {
  if (scanning_) {
    return;
  }

  album_queue_.clear();
  pending_requests_.clear();
  songs_.clear();
  scanning_ = true;
  GetAlbumList(0);
}

void SubsonicLibraryScanner::OnGetAlbumListFinished(QNetworkReply* reply,
                                                    int offset) {
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
    // Start up the maximum number of concurrent requests, finished requests get
    // replaced with new ones
    for (int i = 0; i < kConcurrentRequests && !album_queue_.empty(); ++i) {
      GetAlbum(album_queue_.dequeue());
    }
  }
}

void SubsonicLibraryScanner::OnGetAlbumFinished(QNetworkReply* reply) {
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
    song.set_disc(reader.attributes().value("discNumber").toString().toInt());
    song.set_artist(reader.attributes().value("artist").toString());
    song.set_albumartist(album_artist);
    song.set_bitrate(reader.attributes().value("bitRate").toString().toInt());
    song.set_year(reader.attributes().value("year").toString().toInt());
    song.set_genre(reader.attributes().value("genre").toString());
    qint64 length = reader.attributes().value("duration").toString().toInt();
    length *= kNsecPerSec;
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
  if (!album_queue_.empty()) {
    GetAlbum(album_queue_.dequeue());
  }

  // If this was the last response, we're done!
  if (album_queue_.empty() && pending_requests_.empty()) {
    scanning_ = false;
    emit ScanFinished();
  }
}

void SubsonicLibraryScanner::GetAlbumList(int offset) {
  QUrl url = service_->BuildRequestUrl("getAlbumList2");
  url.addQueryItem("type", "alphabeticalByName");
  url.addQueryItem("size", QString::number(kAlbumChunkSize));
  url.addQueryItem("offset", QString::number(offset));
  QNetworkReply* reply = service_->Send(url);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(OnGetAlbumListFinished(QNetworkReply*, int)), reply, offset);
}

void SubsonicLibraryScanner::GetAlbum(const QString& id) {
  QUrl url = service_->BuildRequestUrl("getAlbum");
  url.addQueryItem("id", id);
  QNetworkReply* reply = service_->Send(url);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(OnGetAlbumFinished(QNetworkReply*)), reply);
  pending_requests_.insert(reply);
}
