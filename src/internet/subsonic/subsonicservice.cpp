/* This file is part of Clementine.
   Copyright 2011-2013, Alan Briolat <alan.briolat@gmail.com>
   Copyright 2013, David Sansome <me@davidsansome.com>
   Copyright 2013, Ross Wolfson <ross.wolfson@gmail.com>
   Copyright 2013-2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Chocobozzz <florian.bigard@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "subsonicservice.h"

#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QSslConfiguration>
#include <QXmlStreamReader>
#include <QUrlQuery>

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
#include "internet/core/internetmodel.h"
#include "internet/subsonic/subsonicurlhandler.h"
#include "internet/subsonic/subsonicdynamicplaylist.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/querygenerator.h"
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
      redirect_count_(0),
      is_ampache_(false) {
  app_->player()->RegisterUrlHandler(url_handler_);

  connect(scanner_, SIGNAL(ScanFinished()), SLOT(ReloadDatabaseFinished()));

  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(), kSongsTable, QString::null,
                         QString::null, kFtsTable);
  connect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(UpdateTotalSongCount(int)));

  using smart_playlists::Generator;
  using smart_playlists::GeneratorPtr;

  library_model_ = new LibraryModel(library_backend_, app_, this);
  library_model_->set_show_various_artists(false);
  library_model_->set_show_smart_playlists(true);
  library_model_->set_default_smart_playlists(
    LibraryModel::DefaultGenerators()
    << (LibraryModel::GeneratorList()
        << GeneratorPtr(new SubsonicDynamicPlaylist(
                          tr("Newest"),
                          SubsonicDynamicPlaylist::QueryStat_Newest))
        << GeneratorPtr(new SubsonicDynamicPlaylist(
                          tr("Random"),
                          SubsonicDynamicPlaylist::QueryStat_Random))
        << GeneratorPtr(new SubsonicDynamicPlaylist(
                          tr("Frequently Played"),
                          SubsonicDynamicPlaylist::QueryStat_Frequent))
        << GeneratorPtr(new SubsonicDynamicPlaylist(
                          tr("Top Rated"),
                          SubsonicDynamicPlaylist::QueryStat_Highest))
        << GeneratorPtr(new SubsonicDynamicPlaylist(
                          tr("Recently Played"),
                          SubsonicDynamicPlaylist::QueryStat_Recent))
        << GeneratorPtr(new SubsonicDynamicPlaylist(
                          tr("Starred"),
                          SubsonicDynamicPlaylist::QueryStat_Starred))
      ));

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
  context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                           tr("Refresh catalogue"), this,
                           SLOT(ReloadDatabase()));
  QAction* config_action = context_menu_->addAction(
      IconLoader::Load("configure", IconLoader::Base), tr("Configure Subsonic..."),
      this, SLOT(ShowConfig()));
  context_menu_->addSeparator();
  context_menu_->addMenu(library_filter_->menu());

  library_filter_->AddMenuAction(config_action);

  app_->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_, tr("Subsonic"), "subsonic",
      IconLoader::Load("subsonic", IconLoader::Provider), true, app_, this));
}

SubsonicService::~SubsonicService() {}

QStandardItem* SubsonicService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("subsonic", IconLoader::Provider),
                            kServiceName);
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

bool SubsonicService::IsAmpache() const { return is_ampache_; }

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
  QUrlQuery url_query;
  url_query.addQueryItem("v", kApiVersion);
  url_query.addQueryItem("c", kApiClientName);
  url_query.addQueryItem("u", username_);
  url_query.addQueryItem("p", QString("enc:" + password_.toUtf8().toHex()));
  url.setQuery(url_query);
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
  // TODO(Alan Briolat): library refresh logic?
  if (newstate != LoginState_Loggedin) library_backend_->DeleteAll();
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
    is_ampache_ = (reader.attributes().value("type") == "ampache");
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
const int SubsonicLibraryScanner::kCoverArtSize = 1024;

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

  bool skip_read_albums = false;

  QXmlStreamReader reader(reply);
  reader.readNextStartElement();

  if (reader.name() != "subsonic-response") {
    ParsingError("Not a subsonic-response. Aborting scan.");
    return;
  }

  if (reader.attributes().value("status") != "ok") {
    reader.readNextStartElement();
    int error = reader.attributes().value("code").toString().toInt();

    // Compatibility with Ampache :
    // When there is no data, Ampache returns NotFound
    // whereas Subsonic returns empty albumList2 tag
    switch (error) {
      case SubsonicService::ApiError_NotFound:
        skip_read_albums = true;
        break;
      default:
        ParsingError("Response status not ok. Aborting scan.");
        return;
    }
  }

  int albums_added = 0;
  if (!skip_read_albums) {
    reader.readNextStartElement();
    if (reader.name() != "albumList2") {
      ParsingError("albumList2 tag expected. Aborting scan.");
      return;
    }

    while (reader.readNextStartElement()) {
      if (reader.name() != "album") {
        ParsingError("album tag expected. Aborting scan.");
        return;
      }

      album_queue_ << reader.attributes().value("id").toString();
      albums_added++;
      reader.skipCurrentElement();
    }
  }

  if (albums_added > 0) {
    // Non-empty reply means potentially more albums to fetch
    GetAlbumList(offset + kAlbumChunkSize);
  } else if (album_queue_.size() == 0) {
    // Empty reply and no albums means an empty Subsonic server
    scanning_ = false;
    emit ScanFinished();
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

  if (reader.name() != "subsonic-response") {
    ParsingError("Not a subsonic-response. Aborting scan.");
    return;
  }

  if (reader.attributes().value("status") != "ok") {
    // TODO(Alan Briolat): error handling
    return;
  }

  // Read album information
  reader.readNextStartElement();
  if (reader.name() != "album") {
    ParsingError("album tag expected. Aborting scan.");
    return;
  }

  QString album_artist = reader.attributes().value("artist").toString();

  // Read song information
  while (reader.readNextStartElement()) {
    if (reader.name() != "song") {
      ParsingError("song tag expected. Aborting scan.");
      return;
    }

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
    QUrl cover_url = service_->BuildRequestUrl("getCoverArt");
    QUrlQuery cover_url_query(url.query());
    cover_url_query.addQueryItem("id", id);
    cover_url.setQuery(cover_url_query);
    song.set_art_automatic(cover_url.toEncoded());
    song.set_url(url);
    song.set_filesize(reader.attributes().value("size").toString().toInt());
    // We need to set these to satisfy the database constraints
    song.set_directory_id(0);
    song.set_mtime(0);
    song.set_ctime(0);

    if (reader.attributes().hasAttribute("playCount")) {
      song.set_playcount(
          reader.attributes().value("playCount").toString().toInt());
    }

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
  QUrlQuery url_query(url.query());
  url_query.addQueryItem("type", "alphabeticalByName");
  url_query.addQueryItem("size", QString::number(kAlbumChunkSize));
  url_query.addQueryItem("offset", QString::number(offset));
  url.setQuery(url_query);
  QNetworkReply* reply = service_->Send(url);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(OnGetAlbumListFinished(QNetworkReply*, int)), reply, offset);
}

void SubsonicLibraryScanner::GetAlbum(const QString& id) {
  QUrl url = service_->BuildRequestUrl("getAlbum");
  QUrlQuery url_query(url.query());
  url_query.addQueryItem("id", id);
  if (service_->IsAmpache()) {
    url_query.addQueryItem("ampache", "1");
  }
  url.setQuery(url_query);
  QNetworkReply* reply = service_->Send(url);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(OnGetAlbumFinished(QNetworkReply*)), reply);
  pending_requests_.insert(reply);
}

void SubsonicLibraryScanner::ParsingError(const QString& message) {
  qLog(Warning) << "Subsonic parsing error: " << message;
  scanning_ = false;
  emit ScanFinished();
}
