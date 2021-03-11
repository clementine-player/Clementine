/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#include "radiobrowserservice.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtDebug>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/radiobrowsersearchprovider.h"
#include "ui/iconloader.h"

bool operator<(const RadioBrowserService::Stream& a,
               const RadioBrowserService::Stream& b) {
  return a.name_.compare(b.name_, Qt::CaseInsensitive) < 0;
}

const char* RadioBrowserService::kServiceName = "Radio-Browser.info";
const char* RadioBrowserService::kSettingsGroup = "RadioBrowser";

QString RadioBrowserService::SearchUrl = "%1/json/stations/byname/%2?limit=%3";
QString RadioBrowserService::PlayClickUrl = "%1/json/url/%2";

QList<RadioBrowserService::Branch> RadioBrowserService::BranchList = {
    {"By Country", "%1/json/countries",
     "%1/json/stations/bycountryexact/%2?hidebroken=true", Type_Category},
    {"By Language", "%1/json/languages",
     "%1/json/stations/bylanguageexact/%2?hidebroken=true", Type_Category},
    {"By Tag", "%1/json/tags", "%1/json/stations/bytagexact/%2?hidebroken=true",
     Type_Category},
    {"By Codec", "%1/json/codecs",
     "%1/json/stations/bycodecexact/%2?hidebroken=true", Type_Category},
    {"Top 100 Clicked", "",
     "%1/json/stations/search?order=clickcount&reverse=true&limit=100",
     Type_Top100},
    {"Top 100 Voted", "",
     "%1/json/stations/search?order=votes&reverse=true&limit=100", Type_Top100},
    {"Top 100 Trending", "",
     "%1/json/stations/search?order=clicktrend&reverse=true&limit=100",
     Type_Top100}};

RadioBrowserService::RadioBrowserService(Application* app,
                                         InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      context_menu_(nullptr),
      network_(new NetworkAccessManager(this)),
      name_(kServiceName),
      homepage_url_(QUrl("https://www.radio-browser.info")),
      icon_(IconLoader::Load("radiobrowser", IconLoader::Provider)) {
  app_->global_search()->AddProvider(
      new RadioBrowserSearchProvider(app_, this, this));
}

RadioBrowserService::~RadioBrowserService() { delete context_menu_; }

QStandardItem* RadioBrowserService::CreateRootItem() {
  root_ = new QStandardItem(icon_, name_);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void RadioBrowserService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      RefreshRootItem();
      break;
    case RadioBrowserService::Type_Category:
      RefreshCategory(item);
      break;
    case RadioBrowserService::Type_CategoryItem:
      RefreshCategoryItem(item);
      break;
    case RadioBrowserService::Type_Top100:
      RefreshTop100(item);
      break;
    default:
      break;
  }
}

void RadioBrowserService::RefreshRootItem() {
  if (!EnsureServerConfig()) {
    ShowConfig();
    return;
  }

  if (root_->hasChildren()) root_->removeRows(0, root_->rowCount());
  for (auto branch : RadioBrowserService::BranchList) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(branch.name);
    item->setData(branch.type, InternetModel::Role_Type);
    item->setData(branch.listUrl, RadioBrowserService::Role_ListUrl);
    item->setData(branch.itemsUrl, RadioBrowserService::Role_ItemsUrl);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    root_->appendRow(item);
  }
}

void RadioBrowserService::RefreshCategory(QStandardItem* item) {
  QString determinedUrl = item->data(RadioBrowserService::Role_ListUrl)
                              .toString()
                              .arg(main_server_url_);
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshCategoryFinished(QNetworkReply*, int, QStandardItem*)),
             reply, task_id, item);
}

void RadioBrowserService::RefreshCategoryItem(QStandardItem* item) {
  QStandardItem* parent = item->parent();
  QString determinedUrl = parent->data(RadioBrowserService::Role_ItemsUrl)
                              .toString()
                              .arg(main_server_url_, item->text());
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshStreamsFinished(QNetworkReply*, int, QStandardItem*)),
             reply, task_id, item);
}

void RadioBrowserService::RefreshTop100(QStandardItem* item) {
  QString determinedUrl = item->data(RadioBrowserService::Role_ItemsUrl)
                              .toString()
                              .arg(main_server_url_);
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshStreamsFinished(QNetworkReply*, int, QStandardItem*)),
             reply, task_id, item);
}

void RadioBrowserService::ShowContextMenu(const QPoint& global_pos) {
  if (!model()->current_index().isValid()) return;
  QStandardItem* item = model()->itemFromIndex(model()->current_index());
  if (!item) return;

  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                             tr("Open %1 in browser").arg(homepage_url_.host()),
                             this, SLOT(Homepage()));

    context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                             tr("Refresh channels"), this,
                             SLOT(ReloadSettings()));
    context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                             tr("Configure..."), this, SLOT(ShowConfig()));
  }

  context_menu_->popup(global_pos);
}

void RadioBrowserService::RefreshCategoryFinished(QNetworkReply* reply,
                                                  int task_id,
                                                  QStandardItem* item) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();
  QJsonDocument document = ParseJsonReply(reply);

  QStringList list;
  QJsonArray contents = document.array();
  qLog(Debug) << "RadioBrowser station list found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonObject item = c.toObject();
    list << item["name"].toString();
  }

  PopulateCategory(item, list);
}

void RadioBrowserService::RefreshStreamsFinished(QNetworkReply* reply,
                                                 int task_id,
                                                 QStandardItem* item) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();
  QJsonDocument document = ParseJsonReply(reply);

  StreamList list;
  QJsonArray contents = document.array();
  qLog(Debug) << "RadioBrowser station list found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonObject item = c.toObject();
    ReadStation(item, &list);
  }

  PopulateStreams(item, list);
}

void RadioBrowserService::ReadStation(QJsonObject& item, StreamList* ret) {
  Stream stream;
  stream.name_ = item["name"].toString();
  stream.uuid_ = item["stationuuid"].toString();
  QUrl url(item["url"].toString());
  stream.url_ = url;
  QUrl favicon(item["favicon"].toString());
  stream.favicon_ = favicon;
  ret->append(stream);
}

Song RadioBrowserService::Stream::ToSong(const QString& prefix) const {
  QString song_title = name_.trimmed();
  if (!song_title.startsWith(prefix)) {
    song_title = prefix + " " + song_title;
  }

  Song ret;
  ret.set_valid(true);
  ret.set_title(song_title);
  ret.set_artist(name_);
  ret.set_url(url_);
  ret.set_art_automatic(favicon_.toString());
  return ret;
}

void RadioBrowserService::Homepage() {
  QDesktopServices::openUrl(homepage_url_);
}

PlaylistItem::Options RadioBrowserService::playlistitem_options() const {
  return PlaylistItem::PauseDisabled | PlaylistItem::SeekDisabled;
}

void RadioBrowserService::PopulateCategory(QStandardItem* parentItem,
                                           QStringList& elements) {
  if (parentItem->hasChildren())
    parentItem->removeRows(0, parentItem->rowCount());

  for (const QString& element : elements) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(element);
    item->setData(RadioBrowserService::Type_CategoryItem,
                  InternetModel::Role_Type);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    parentItem->appendRow(item);
  }
}

void RadioBrowserService::PopulateStreams(QStandardItem* parentItem,
                                          StreamList& streams) {
  if (parentItem->hasChildren())
    parentItem->removeRows(0, parentItem->rowCount());

  for (const Stream& stream : streams) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(stream.name_);
    item->setData(QVariant::fromValue(stream.ToSong(name_)),
                  InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem,
                  InternetModel::Role_PlayBehaviour);
    item->setData(stream.uuid_, RadioBrowserService::Role_StationUuid);

    parentItem->appendRow(item);
  }
}

QDataStream& operator<<(QDataStream& out,
                        const RadioBrowserService::Stream& stream) {
  out << stream.name_ << stream.url_;
  return out;
}

QDataStream& operator>>(QDataStream& in, RadioBrowserService::Stream& stream) {
  in >> stream.name_ >> stream.url_;
  return in;
}

void RadioBrowserService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  main_server_url_ = s.value("server").toString();

  if (root_ && root_->hasChildren()) {
    root_->removeRows(0, root_->rowCount());
    root_->setData(true, InternetModel::Role_CanLazyLoad);
  }
}

bool RadioBrowserService::EnsureServerConfig() {
  if (main_server_url_.isEmpty())
    return false;
  else
    return true;
}

void RadioBrowserService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_RadioBrowser);
}

void RadioBrowserService::Search(int search_id, const QString& query,
                                 const int limit) {
  QString determinedUrl =
      RadioBrowserService::SearchUrl.arg(main_server_url_, query).arg(limit);
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SearchFinishedInternal(QNetworkReply*, int, int)), reply,
             task_id, search_id);
}

void RadioBrowserService::SearchFinishedInternal(QNetworkReply* reply,
                                                 int task_id, int search_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();
  QJsonDocument document = ParseJsonReply(reply);

  StreamList list;
  QJsonArray contents = document.array();
  qLog(Debug) << "RadioBrowser station list found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonObject item = c.toObject();
    ReadStation(item, &list);
  }

  emit SearchFinished(search_id, list);
}

void RadioBrowserService::ItemNowPlaying(QStandardItem* item) {
  QString station_uuid =
      item->data(RadioBrowserService::Role_StationUuid).toString();
  if (station_uuid.isEmpty()) return;

  QString determinedUrl =
      RadioBrowserService::PlayClickUrl.arg(main_server_url_, station_uuid);
  QUrl url(determinedUrl);

  qLog(Debug) << "RadioBrowser station played:" << determinedUrl;
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished,
          [this, reply]() { reply->deleteLater(); });
}