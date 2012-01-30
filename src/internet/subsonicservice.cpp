#include "subsonicurlhandler.h"
#include "subsonicservice.h"
#include "internetmodel.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/utilities.h"
#include "ui/iconloader.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QSslConfiguration>
#include <QXmlStreamReader>

const char* SubsonicService::kServiceName = "Subsonic";
const char* SubsonicService::kSettingsGroup = "Subsonic";
const char* SubsonicService::kApiVersion = "1.7.0";
const char* SubsonicService::kApiClientName = "Clementine";

SubsonicService::SubsonicService(InternetModel *parent)
  : InternetService(kServiceName, parent, parent),
    network_(new QNetworkAccessManager(this)),
    http_url_handler_(new SubsonicUrlHandler(this, this)),
    https_url_handler_(new SubsonicHttpsUrlHandler(this, this)),
    login_state_(LoginState_OtherError),
    item_lookup_()
{
  model()->player()->RegisterUrlHandler(http_url_handler_);
  model()->player()->RegisterUrlHandler(https_url_handler_);
  connect(this, SIGNAL(LoginStateChanged(SubsonicService::LoginState)),
          SLOT(onLoginStateChanged(SubsonicService::LoginState)));
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
  // The "root" item
  case InternetModel::Type_Service:
    GetIndexes();
    break;

  // Any folder item
  case InternetModel::Type_UserPlaylist:
    qLog(Debug) << "Lazy loading" << item->data(Role_Id).toString();
    GetMusicDirectory(item->data(Role_Id).toString());
    break;

  default:
    break;
  }

  item->setRowCount(0);
  QStandardItem* loading = new QStandardItem(tr("Loading..."));
  item->appendRow(loading);
}

smart_playlists::GeneratorPtr SubsonicService::CreateGenerator(QStandardItem* item)
{
  qLog(Debug) << "Attempting to smart load" << item->data(Role_Id).toString();
  return smart_playlists::GeneratorPtr();
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
  Send(BuildRequestUrl("ping"), SLOT(onPingFinished()));
}

void SubsonicService::GetIndexes()
{
  Send(BuildRequestUrl("getIndexes"), SLOT(onGetIndexesFinished()));
}

void SubsonicService::GetMusicDirectory(const QString &id)
{
  QUrl url = BuildRequestUrl("getMusicDirectory");
  url.addQueryItem("id", id);
  Send(url, SLOT(onGetMusicDirectoryFinished()));
}

QModelIndex SubsonicService::GetCurrentIndex()
{
  return context_item_;
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

void SubsonicService::Send(const QUrl &url, const char *slot)
{
  QNetworkRequest request(url);
  // Don't try and check the authenticity of the SSL certificate - it'll almost
  // certainly be self-signed.
  QSslConfiguration sslconfig = QSslConfiguration::defaultConfiguration();
  sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
  request.setSslConfiguration(sslconfig);
  QNetworkReply *reply = network_->get(request);
  connect(reply, SIGNAL(finished()), slot);
}

void SubsonicService::ReadIndex(QXmlStreamReader *reader, QStandardItem *parent)
{
  Q_ASSERT(reader->name() == "index");

  while (reader->readNextStartElement())
  {
    ReadArtist(reader, parent);
  }
}

void SubsonicService::ReadArtist(QXmlStreamReader *reader, QStandardItem *parent)
{
  Q_ASSERT(reader->name() == "artist");
  QString id = reader->attributes().value("id").toString();
  QStandardItem *item = new QStandardItem(IconLoader::Load("document-open-folder"),
                                          reader->attributes().value("name").toString());
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
  item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
  item->setData(id, Role_Id);
  parent->appendRow(item);
  item_lookup_.insert(id, item);
  reader->skipCurrentElement();
}

void SubsonicService::ReadAlbum(QXmlStreamReader *reader, QStandardItem *parent)
{
  Q_ASSERT(reader->name() == "child");
  QString id = reader->attributes().value("id").toString();
  QStandardItem *item = new QStandardItem(IconLoader::Load("document-open-folder"),
                                          reader->attributes().value("title").toString());
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::Type_UserPlaylist, InternetModel::Role_Type);
  item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
  item->setData(id, Role_Id);
  parent->appendRow(item);
  item_lookup_.insert(id, item);
  reader->skipCurrentElement();
}

void SubsonicService::ReadTrack(QXmlStreamReader *reader, QStandardItem *parent)
{
  Q_ASSERT(reader->name() == "child");

  Song song;
  QString id = reader->attributes().value("id").toString();
  song.set_title(reader->attributes().value("title").toString());
  song.set_album(reader->attributes().value("album").toString());
  song.set_track(reader->attributes().value("track").toString().toInt());
  song.set_artist(reader->attributes().value("artist").toString());
  song.set_bitrate(reader->attributes().value("bitRate").toString().toInt());
  song.set_year(reader->attributes().value("year").toString().toInt());
  song.set_genre(reader->attributes().value("genre").toString());
  qint64 length = reader->attributes().value("duration").toString().toInt();
  length *= 1000000000;
  song.set_length_nanosec(length);
  QUrl url = BuildRequestUrl("stream");
  url.setScheme(url.scheme() == "https" ? "subsonics" : "subsonic");
  url.addQueryItem("id", id);
  song.set_url(url);
  song.set_filesize(reader->attributes().value("size").toString().toInt());

  QStandardItem *item = new QStandardItem(reader->attributes().value("title").toString());
  item->setData(Type_Track, InternetModel::Role_Type);
  item->setData(id, Role_Id);
  item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
  item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
  item->setData(song.url(), InternetModel::Role_Url);
  parent->appendRow(item);
  item_lookup_.insert(id, item);
  reader->skipCurrentElement();
}

void SubsonicService::onLoginStateChanged(SubsonicService::LoginState newstate)
{
  root_->setRowCount(0);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
}

void SubsonicService::onPingFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
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

void SubsonicService::onGetIndexesFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();
  QXmlStreamReader reader(reply);

  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "subsonic-response");
  if (reader.attributes().value("status") != "ok")
  {
    // TODO: error handling
    return;
  }

  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "indexes");
  root_->setRowCount(0);
  while (reader.readNextStartElement())
  {
    if (reader.name() == "index")
    {
      ReadIndex(&reader, root_);
    }
    else if (reader.name() == "child" && reader.attributes().value("isVideo") == "false")
    {
      ReadTrack(&reader, root_);
    }
    else
    {
      reader.skipCurrentElement();
    }
  }
}

void SubsonicService::onGetMusicDirectoryFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();
  QXmlStreamReader reader(reply);

  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "subsonic-response");
  if (reader.attributes().value("status") != "ok")
  {
    // TODO: error handling
    return;
  }

  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "directory");
  QStandardItem *parent = item_lookup_.value(reader.attributes().value("id").toString());
  parent->setRowCount(0);
  while (reader.readNextStartElement())
  {
    if (reader.attributes().value("isDir") == "true")
    {
      ReadAlbum(&reader, parent);
    }
    else if (reader.attributes().value("isVideo") == "false")
    {
      ReadTrack(&reader, parent);
    }
    else
    {
      reader.skipCurrentElement();
    }
  }
}
