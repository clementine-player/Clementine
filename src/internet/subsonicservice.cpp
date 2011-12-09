#include "subsonicservice.h"
#include "internetmodel.h"
#include "core/logging.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QXmlStreamReader>

const char* SubsonicService::kServiceName = "Subsonic";
const char* SubsonicService::kSettingsGroup = "Subsonic";
const char* SubsonicService::kApiVersion = "1.6.0";
const char* SubsonicService::kApiClientName = "Clementine";

SubsonicService::SubsonicService(InternetModel *parent)
  : InternetService(kServiceName, parent, parent),
    network_(new QNetworkAccessManager(this)),
    login_state_(LoginState_OtherError)
{
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
    GetMusicFolders();
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
  Send(BuildRequestUrl("ping"), SLOT(onPingFinished()));
}

void SubsonicService::GetMusicFolders()
{
  Send(BuildRequestUrl("getMusicFolders"), SLOT(onGetMusicFoldersFinished()));
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
  QNetworkReply *reply = network_->get(QNetworkRequest(url));
  // It's very unlikely the Subsonic server will have a valid SSL certificate
  reply->ignoreSslErrors();
  connect(reply, SIGNAL(finished()), slot);
}

void SubsonicService::onPingFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError)
  {
    login_state_ = LoginState_BadServer;
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
      switch (error)
      {
      // "Parameter missing" for "ping" is always blank username or password
      case ApiError_ParameterMissing:
      case ApiError_BadCredentials:
        login_state_ = LoginState_BadCredentials;
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
  qLog(Debug) << "Login state changed: " << login_state_;
  emit LoginStateChanged(login_state_);
}

void SubsonicService::onGetMusicFoldersFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();
  QXmlStreamReader reader(reply);

  reader.readNextStartElement();
  if (reader.attributes().value("status") != "ok")
  {
    // TODO: error handling
    return;
  }

  reader.readNextStartElement();
  Q_ASSERT(reader.name() == "musicFolders");
  while (reader.readNextStartElement())
  {
    QStandardItem *item = new QStandardItem(reader.attributes().value("name").toString());
    item->setData(Type_TopLevel, InternetModel::Role_Type);
    root_->appendRow(item);
    reader.skipCurrentElement();
  }
}
