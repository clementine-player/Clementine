/* This file is part of Clementine.
   Copyright 2015, Marco Kirchner <kirchnermarco@gmail.com>
   Copyright 2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, maximko <me@maximko.org>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "playmusicservice.h"

#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "internet/core/searchboxwidget.h"

#include "core/application.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "core/player.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/playmusicsearchprovider.h"
#include "playmusicurlhandler.h"
#include "ui/iconloader.h"


const char* PlayMusicService::kS1 = "VzeC4H4h+T2f0VI180nVX8x+Mb5HiTtGnKgH52Otj8ZCGDz9jRW"
        "yHb6QXK0JskSiOgzQfwTY5xgLLSdUSreaLVMsVVWfxfa8Rw==";
const char* PlayMusicService::kS2 = "ZAPnhUkYwQ6y5DdQxWThbvhJHN8msQ1rqJw0ggKdufQjelrKuiG"
        "GJI30aswkgCWTDyHkTGK9ynlqTkJ5L4CiGGUabGeo8M6JTQ==";

const char* PlayMusicService::kServiceName = "Google Play Music";
const char* PlayMusicService::kSettingsGroup = "Play Music";

const char* PlayMusicService::kBaseUrl = "https://www.googleapis.com/sj/v1/";
const char* PlayMusicService::kWebUrl = "https://play.google.com/music/";
const char* PlayMusicService::kAuthUrl = "https://www.google.com/accounts/ClientLogin";

const int PlayMusicService::kSearchDelayMsec = 400;
const int PlayMusicService::kSongSearchLimit = 100;

typedef QPair<QString, QString> Param;

PlayMusicService::PlayMusicService(Application* app, InternetModel* parent)
        : InternetService(kServiceName, app, parent, parent),
          root_(nullptr),
          search_(nullptr),
          network_(new NetworkAccessManager(this)),
          context_menu_(nullptr),
          search_box_(new SearchBoxWidget(this)),
          search_delay_(new QTimer(this)),
          next_pending_search_id_(0) {
  qsrand(QTime::currentTime().msec());

  QByteArray s1 = QByteArray::fromBase64(kS1);
  QByteArray s2 = QByteArray::fromBase64(kS2);
  QByteArray key = QByteArray(s1.size(), '\0');
  for (int i = 0; i < s1.length(); i++) {
    key[i] = s1[i]^s2[i];
  }
  key_ = QString(key);

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));

  PlayMusicSearchProvider* search_provider =
          new PlayMusicSearchProvider(app_, this);
  search_provider->Init(this);
  app_->global_search()->AddProvider(search_provider);

  connect(search_box_, SIGNAL(TextChanged(QString)), SLOT(Search(QString)));
  app->player()->RegisterUrlHandler(new PlayMusicUrlHandler(this, this));
}

PlayMusicService::~PlayMusicService() {}

QStandardItem* PlayMusicService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/playmusic.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  root_->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                 InternetModel::Role_PlayBehaviour);
  return root_;
}

void PlayMusicService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service: {
      EnsureItemsCreated();
      break;
    }
    default:
      break;
  }
}

void PlayMusicService::EnsureItemsCreated() {
  if (!search_) {
    search_ =
            new QStandardItem(IconLoader::Load("edit-find"), tr("Search results"));
    search_->setToolTip(
            tr("Start typing something on the search box above to "
                       "fill this search results list"));
    search_->setData(InternetModel::PlayBehaviour_MultipleItems,
                     InternetModel::Role_PlayBehaviour);
    root_->appendRow(search_);
  }
}

QWidget* PlayMusicService::HeaderWidget() const { return search_box_; }

void PlayMusicService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_PlayMusic);
}

void PlayMusicService::Homepage() {
  QDesktopServices::openUrl(QUrl("https://play.google.com/"));
}

void PlayMusicService::Login(const QString &username, const QString &password) {
  QUrl params;
  params.addEncodedQueryItem("accountType", "HOSTED_OR_GOOGLE");
  params.addEncodedQueryItem("Email", username.toUtf8());
  params.addEncodedQueryItem("Passwd", QUrl::toPercentEncoding(password));
  params.addEncodedQueryItem("service", "sj");
  params.addEncodedQueryItem("source", "clementine-playmusic-0.1");
  QByteArray paramBytes = params.toString().mid(1).toUtf8();

  QUrl authUrl = QUrl(QString(kAuthUrl));
  QNetworkRequest request(authUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

  QNetworkReply *reply = network_->post(request, paramBytes);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LoginFinished(QNetworkReply*)), reply);
}

void PlayMusicService::LoginFinished(QNetworkReply *reply) {
  reply->deleteLater();

  QByteArray data = reply->readAll();
  QString result(data);
  QRegExp rx("Auth=(.[^\\n\\r]+)");
  if (rx.indexIn(result, 0) != -1) {
    this->access_token_ = rx.cap(1);
  } else {
    qLog(Warning) << "PlayMusic: Error getting AccessToken";
    this->access_token_ = QString();
  }
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("access_token", this->access_token_);
  LoginStateChanged();
}

void PlayMusicService::LoadAccessTokenIfEmpty() {
  if (access_token_.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    if (s.contains("access_token")) {
      access_token_ = s.value("access_token").toString();
    }
  }
  if (device_id_.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    if (s.contains("device_id")) {
      device_id_ = s.value("device_id").toString();
    }
  }
  if (xt_.isEmpty()) {
    LoadWebToken();
  }
}

void PlayMusicService::LoadWebToken() {
  QUrl url(QString(kWebUrl) + "listen");

  QNetworkRequest req(url);
  req.setRawHeader("Authorization", QString("GoogleLogin auth=%1").arg(this->access_token_).toUtf8());
  QNetworkReply* reply = network_->head(req);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LoadWebTokenFinished(QNetworkReply*)), reply);
}

void PlayMusicService::LoadWebTokenFinished(QNetworkReply *reply) {
  reply->deleteLater();

  QString cookie = QString(reply->rawHeader("Set-Cookie"));
  QRegExp rx("xt=(.[^;]+)");
  if (rx.indexIn(cookie, 0) != -1) {
    this->xt_ = rx.cap(1);
  } else {
    qLog(Warning) << "PlayMusic: Error getting WebToken";
    this->xt_ = QString();
  }

  if (device_id_.isEmpty()) {
    LoadDeviceId();
  }
}

void PlayMusicService::LoadDeviceId() {
  device_id_ = QString();

  QUrl params;
  params.addEncodedQueryItem("u", "0");
  params.addEncodedQueryItem("xt", QUrl::toPercentEncoding(xt_));
  QByteArray paramBytes = params.toString().mid(1).toUtf8();

  QUrl authUrl = QUrl(QString(kWebUrl) + "services/loadsettings");
  QNetworkRequest request(authUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  request.setRawHeader("Authorization", QString("GoogleLogin auth=%1").arg(this->access_token_).toUtf8());

  QNetworkReply *reply = network_->post(request, paramBytes);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LoadDeviceIdFinished(QNetworkReply*)), reply);
}

void PlayMusicService::LoadDeviceIdFinished(QNetworkReply *reply) {
  reply->deleteLater();

  QVariantMap response = ExtractResult(reply).toMap();
  QVariantMap settings = response["settings"].toMap();
  if (settings.isEmpty()) {
    QMessageBox::warning(0, tr("Error getting device id"), tr("Couldn't access account settings"));
    Logout();
    return;
  }
  bool allAccess = settings["isSubscription"].toBool();
  if (!allAccess) {
    QMessageBox::warning(0, tr("No AllAccess subscription"),
                         tr("You need an AllAccess subscription for this service"));
    Logout();
    return;
  }
  QVariantList devices = settings["devices"].toList();
  for(const QVariant &device : devices) {
    QVariantMap deviceMap = device.toMap();
    if (deviceMap["type"].toString() == "PHONE") {
      device_id_ = deviceMap["id"].toString().mid(2);
      break;
    } else if (deviceMap["type"].toString() == "IOS") {
      device_id_ = deviceMap["id"].toString();
      break;
    }
  }
  if (this->device_id_.isEmpty()) {
    QMessageBox::warning(0, tr("No device registered"),
                         tr("There are no Android/iOS devices registered to this account."
                                    "To register a device, you'll need to open the Play Music app on your device and log in with your account."));
    Logout();
    return;
  }
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("device_id", this->device_id_);
}

bool PlayMusicService::IsLoggedIn() {
  LoadAccessTokenIfEmpty();
  return !access_token_.isEmpty();
}

void PlayMusicService::Logout() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  access_token_.clear();
  s.remove("access_token");
  s.remove("device_id");
  LoginStateChanged();
}

QString PlayMusicService::GenerateSalt(int len) {
  const QString possibleCharacters("abcdefghijklmnopqrstuvwxyz0123456789");

  QString randomString;
  for(int i=0; i<len; ++i)
  {
    int index = qrand() % possibleCharacters.length();
    QChar nextChar = possibleCharacters.at(index);
    randomString.append(nextChar);
  }
  return randomString;
}

QByteArray PlayMusicService::HmacSHA1(const QByteArray& text, const QByteArray& in_key)
{
    int blocksize = 64;

    QByteArray key(in_key);

    if (key.length() > blocksize) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }
    if (key.length() < blocksize) {
        key += QByteArray(blocksize-key.length(), 0);
    }

    QByteArray o_pad(blocksize, char(0x5c));
    QByteArray i_pad(blocksize, char(0x36));

    for (int i=0; i<blocksize; ++i) {
        o_pad[i] = o_pad[i] ^ key[i];
        i_pad[i] = i_pad[i] ^ key[i];
    }

    return QCryptographicHash::hash(o_pad +
        QCryptographicHash::hash(i_pad + text, QCryptographicHash::Sha1), QCryptographicHash::Sha1);
}

void PlayMusicService::LoadStreamUrl(QString id) {
  LoadAccessTokenIfEmpty();

  if (id[0].isLower()) {
    id[0] = id[0].toUpper();
  }
  QString salt = GenerateSalt(13);

  QString sig = HmacSHA1(QString(id+salt).toAscii(), key_.toAscii()).toBase64()
          .replace("=", "")
          .replace("+","-")
          .replace("/", "_");

  QUrl url = QUrl("https://android.clients.google.com/music/mplay");
  url.addEncodedQueryItem("net", "mob");
  url.addEncodedQueryItem("pt", "e");
  url.addEncodedQueryItem("opt", "hi");
  url.addEncodedQueryItem('T' == id[0] ? "mjck" : "songid", id.toAscii());
  url.addEncodedQueryItem("slt", salt.toAscii());
  url.addEncodedQueryItem("sig", sig.toAscii());
  qDebug() << url.toString();

  QNetworkRequest request(url);
  request.setRawHeader("Authorization", QString("GoogleLogin auth=%1").arg(this->access_token_).toAscii());
  request.setRawHeader("X-Device-ID", device_id_.toAscii());

  QNetworkReply *reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LoadStreamUrlFinished(QNetworkReply*, const QString&)), reply, id);
}

void PlayMusicService::LoadStreamUrlFinished(QNetworkReply *reply, const QString &id) {
  reply->deleteLater();

  QString location = reply->header(QNetworkRequest::LocationHeader).toString();
  if(location.isEmpty()) {
    LoadStreamUrlComplete(QUrl(), id);
    qLog(Warning) << "PlayMusic: Error generating streaming-url for trackid:" << id;
    return;
  }
  LoadStreamUrlComplete(QUrl(location), id);
}

void PlayMusicService::Search(const QString& text, bool now) {
  pending_search_ = text;

  // If there is no text (e.g. user cleared search box), we don't need to do a
  // real query that will return nothing: we can clear the playlist now
  if (text.isEmpty()) {
    search_delay_->stop();
    ClearSearchResults();
    return;
  }

  if (now) {
    search_delay_->stop();
    DoSearch();
  } else {
    search_delay_->start();
  }
}

void PlayMusicService::DoSearch() {
  LoadAccessTokenIfEmpty();
  ClearSearchResults();

  QList<Param> parameters;
  parameters << Param("q", pending_search_);
  parameters << Param("max-results", QString::number(kSongSearchLimit));
  QNetworkReply* reply = CreateRequest("query", parameters);
  const int id = next_pending_search_id_++;
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SearchFinished(QNetworkReply*, int)), reply, id);
}

void PlayMusicService::SearchFinished(QNetworkReply* reply, int task_id) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply));
  // Fill results list
  for (const Song& song : songs) {
    QStandardItem* child = CreateSongItem(song);
    search_->appendRow(child);
  }

  QModelIndex index = model()->merged_model()->mapFromSource(search_->index());
  ScrollToIndex(index);
}

void PlayMusicService::ClearSearchResults() {
  if (search_) {
    search_->removeRows(0, search_->rowCount());
  }
}

int PlayMusicService::SimpleSearch(const QString& text) {
  LoadAccessTokenIfEmpty();
  QList<Param> parameters;
  parameters << Param("q", text);
  parameters << Param("max-results", QString::number(kSongSearchLimit));
  QNetworkReply* reply = CreateRequest("query", parameters);
  const int id = next_pending_search_id_++;
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SimpleSearchFinished(QNetworkReply*, int)), reply, id);
  return id;
}

void PlayMusicService::SimpleSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply));
  emit SimpleSearchResults(id, songs);
}

void PlayMusicService::EnsureMenuCreated() {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download"),
                             tr("Open %1 in browser").arg("Google Play Music"),
                             this, SLOT(Homepage()));
  }
}

void PlayMusicService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  context_menu_->popup(global_pos);
}

QNetworkReply* PlayMusicService::CreateRequest(const QString& ressource_name,
                                               const QList<Param>& params) {
  QUrl url(kBaseUrl + ressource_name);

  for (const Param& param : params) {
    url.addQueryItem(param.first, param.second);
  }

  qLog(Debug) << "Request Url: " << url.toEncoded();

  QNetworkRequest req(url);
  req.setRawHeader("Authorization", QString("GoogleLogin auth=%1").arg(this->access_token_).toUtf8());
  QNetworkReply* reply = network_->get(req);
  return reply;
}

QVariant PlayMusicService::ExtractResult(QNetworkReply* reply) {
  if (reply->error() != QNetworkReply::NoError) {
    qLog(Error) << "Error when retrieving PlayMusic results:"
    << reply->errorString() << QString(" (%1)").arg(reply->error());
    if (reply->error() == QNetworkReply::ContentAccessDenied ||
        reply->error() == QNetworkReply::ContentOperationNotPermittedError ||
        reply->error() == QNetworkReply::ContentNotFoundError ||
        reply->error() == QNetworkReply::AuthenticationRequiredError) {

      // In case of access denied errors (invalid token?) logout
      Logout();
      return QVariant();
    }
  }
  QJson::Parser parser;
  bool ok;
  QVariant result = parser.parse(reply, &ok);
  if (!ok) {
    qLog(Error) << "Error while parsing PlayMusic result";
  }
  return result;
}

SongList PlayMusicService::ExtractSongs(const QVariant& result) {
  SongList songs;

  QVariantList q_variant_list = result.toMap().value("entries").toList();
  for (const QVariant& q : q_variant_list) {
    Song song = ExtractSong(q.toMap());
    if (song.is_valid()) {
      songs << song;
    }
  }
  return songs;
}

Song PlayMusicService::ExtractSong(const QVariantMap& result_song) {
  Song song;
  if (!result_song.isEmpty() && result_song["type"].toInt() == 1) {
    QVariantMap track = result_song["track"].toMap();
    QString id;
    if (track.contains("id"))
      id = track["id"].toString();
    else
      id = track["nid"].toString();

    QUrl stream_url("playmusic://" + id);
    song.set_url(stream_url);

    QString artist = track["artist"].toString();
    song.set_artist(artist);

    QString title = track["title"].toString();
    song.set_title(title);

    QString album = track["album"].toString();
    song.set_album(album);

    QString genre = track["genre"].toString();
    song.set_genre(genre);

    int disc = track["disc"].toInt();
    song.set_disc(disc);

    int year = track["year"].toInt();
    song.set_year(year);

    QVariantList covers = track["albumArtRef"].toList();
    if (covers.size() > 0) {
      QVariantMap cover = covers[0].toMap();
      song.set_art_automatic(cover["url"].toString());
    }

    QVariant q_duration = track["durationMillis"];
    quint64 duration = q_duration.toULongLong() * kNsecPerMsec;
    song.set_length_nanosec(duration);

    song.set_valid(true);
  }
  return song;
}
