#include "subsonicurlhandler.h"
#include "subsonicservice.h"
#include "internetmodel.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/utilities.h"
#include "ui/iconloader.h"
#include "library/librarybackend.h"
#include "core/mergedproxymodel.h"
#include "core/database.h"
#include "core/closure.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QSslConfiguration>
#include <QXmlStreamReader>
#include <QSortFilterProxyModel>

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
    library_backend_(NULL),
    library_model_(NULL),
    library_sort_model_(new QSortFilterProxyModel(this)),
    login_state_(LoginState_OtherError)
{
  app_->player()->RegisterUrlHandler(url_handler_);

  connect(scanner_, SIGNAL(SongsDiscovered(SongList)),
          SLOT(onSongsDiscovered(SongList)));

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

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  connect(this, SIGNAL(LoginStateChanged(SubsonicService::LoginState)),
          SLOT(onLoginStateChanged(SubsonicService::LoginState)));
}

SubsonicService::~SubsonicService()
{
}

QStandardItem* SubsonicService::CreateRootItem()
{
  QStandardItem* item = new QStandardItem(QIcon(":providers/subsonic.png"), kServiceName);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  return item;
}

void SubsonicService::LazyPopulate(QStandardItem *item)
{
  switch (item->data(InternetModel::Role_Type).toInt())
  {
  case InternetModel::Type_Service:
    // TODO: initiate library loading
    library_backend_->DeleteAll();
    scanner_->Scan();
    model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
    break;

  default:
    break;
  }
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

QModelIndex SubsonicService::GetCurrentIndex()
{
  return context_item_;
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

void SubsonicService::onSongsDiscovered(SongList songs)
{
  library_backend_->AddOrUpdateSongs(songs);
}


const int SubsonicLibraryScanner::kAlbumChunkSize = 500;
const int SubsonicLibraryScanner::kSongListMinChunkSize = 500;
const int SubsonicLibraryScanner::kConcurrentRequests = 8;

SubsonicLibraryScanner::SubsonicLibraryScanner(SubsonicService* service, QObject* parent)
  : QObject(parent),
    service_(service)
{
}

SubsonicLibraryScanner::~SubsonicLibraryScanner()
{
}

void SubsonicLibraryScanner::Scan()
{
  album_queue_.clear();
  songlist_buffer_.clear();
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

  // If this reply was non-empty, get the next chunk, otherwise start fetching songs
  if (albums_added > 0) {
    GetAlbumList(offset + kAlbumChunkSize);
  } else {
    // Start up the maximum number of concurrent requests
    for (int i = 0; i < kConcurrentRequests && !album_queue_.empty(); ++i) {
      GetAlbum(album_queue_.dequeue());
    }
  }
}

void SubsonicLibraryScanner::onGetAlbumFinished(QNetworkReply *reply)
{
  reply->deleteLater();

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "subsonic-response");
  if (reader.attributes().value("status") != "ok") {
    // TODO: error handling
    return;
  }

  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "album");
  while (reader.readNextStartElement()) {
    Q_ASSERT(reader.name() == "song");
    Song song;
    QString id = reader.attributes().value("id").toString();
    song.set_title(reader.attributes().value("title").toString());
    song.set_album(reader.attributes().value("album").toString());
    song.set_track(reader.attributes().value("track").toString().toInt());
    song.set_artist(reader.attributes().value("artist").toString());
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
    songlist_buffer_ << song;
    reader.skipCurrentElement();
  }

  // If the songlist buffer is big enough, or we're (nearly) done, emit the songlist (see below)
  bool should_emit = album_queue_.empty() || songlist_buffer_.size() >= kSongListMinChunkSize;

  // Start the next request
  if (!album_queue_.empty()) {
    GetAlbum(album_queue_.dequeue());
  }

  if (should_emit) {
    emit SongsDiscovered(songlist_buffer_);
    songlist_buffer_.clear();
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
}
