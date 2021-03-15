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
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/radiobrowsersearchprovider.h"
#include "internet/internetradio/savedradio.h"
#include "radiobrowserurlhandler.h"
#include "ui/iconloader.h"

Q_DECLARE_METATYPE(QStandardItem*)

namespace {
struct Branch {
  QString name;
  QString listUrl;
  QString itemsUrl;
  RadioBrowserService::Type type;
};

QList<Branch> BranchList = {
    {"By Country", "%1/json/countries",
     "%1/json/stations/bycountryexact/%2?hidebroken=true",
     RadioBrowserService::Type_Category},
    {"By Language", "%1/json/languages",
     "%1/json/stations/bylanguageexact/%2?hidebroken=true",
     RadioBrowserService::Type_Category},
    {"By Tag", "%1/json/tags", "%1/json/stations/bytagexact/%2?hidebroken=true",
     RadioBrowserService::Type_Category},
    {"By Codec", "%1/json/codecs",
     "%1/json/stations/bycodecexact/%2?hidebroken=true",
     RadioBrowserService::Type_Category},
    {"Top 100 Clicked", "",
     "%1/json/stations/search?order=clickcount&reverse=true&limit=100",
     RadioBrowserService::Type_Top100},
    {"Top 100 Voted", "",
     "%1/json/stations/search?order=votes&reverse=true&limit=100",
     RadioBrowserService::Type_Top100},
    {"Top 100 Trending", "",
     "%1/json/stations/search?order=clicktrend&reverse=true&limit=100",
     RadioBrowserService::Type_Top100}};

QString SearchUrl = "%1/json/stations/byname/%2?limit=%3";
QString PlayClickUrl = "%1/json/url/%2";

void ReadStation(const QJsonObject& item, RadioBrowserService::StreamList* ret,
                 RadioBrowserUrlHandler* url_handler_) {
  RadioBrowserService::Stream stream;
  stream.name_ = item["name"].toString();
  QUrl url(item["stationuuid"].toString());
  url.setScheme(url_handler_->scheme());
  stream.url_ = url;
  QUrl favicon(item["favicon"].toString());
  stream.favicon_ = favicon;
  ret->append(stream);
}

void PopulateCategory(QStandardItem* parentItem, QStringList& elements) {
  if (parentItem->hasChildren())
    parentItem->removeRows(0, parentItem->rowCount());

  for (auto element : elements) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(element);
    item->setData(RadioBrowserService::Type_CategoryItem,
                  InternetModel::Role_Type);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    parentItem->appendRow(item);
  }
}

void PopulateStreams(QStandardItem* parentItem,
                     RadioBrowserService::StreamList& streams) {
  if (parentItem->hasChildren())
    parentItem->removeRows(0, parentItem->rowCount());

  for (auto stream : streams) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(stream.name_);
    item->setData(
        QVariant::fromValue(stream.ToSong(RadioBrowserService::kServiceName)),
        InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem,
                  InternetModel::Role_PlayBehaviour);
    item->setData(InternetModel::Type_Track, InternetModel::Role_Type);

    parentItem->appendRow(item);
  }
}
}  // namespace

bool operator<(const RadioBrowserService::Stream& a,
               const RadioBrowserService::Stream& b) {
  return a.name_.compare(b.name_, Qt::CaseInsensitive) < 0;
}

const char* RadioBrowserService::kServiceName = "Radio-Browser.info";
const char* RadioBrowserService::kSettingsGroup = "RadioBrowser";
const char* RadioBrowserService::kSchemeName = "radiobrowser";
const char* RadioBrowserService::defaultServer =
    "http://all.api.radio-browser.info";

RadioBrowserService::RadioBrowserService(Application* app,
                                         InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      context_menu_(nullptr),
      station_menu_(nullptr),
      add_to_saved_radio_action_(nullptr),
      network_(new NetworkAccessManager(this)),
      url_handler_(new RadioBrowserUrlHandler(app, this, this)),
      homepage_url_(QUrl("https://www.radio-browser.info")),
      icon_(IconLoader::Load("radiobrowser", IconLoader::Provider)) {
  app_->player()->RegisterUrlHandler(url_handler_);
  app_->global_search()->AddProvider(
      new RadioBrowserSearchProvider(app_, this, this));
}

QStandardItem* RadioBrowserService::CreateRootItem() {
  root_ = new QStandardItem(icon_, RadioBrowserService::kServiceName);
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
  for (auto branch : BranchList) {
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
  if (!EnsureServerConfig()) {
    ShowConfig();
    return;
  }

  QString determinedUrl = item->data(RadioBrowserService::Role_ListUrl)
                              .toString()
                              .arg(main_server_url_);
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  connect(reply, &QNetworkReply::finished, [=] {
    app_->task_manager()->SetTaskFinished(task_id);
    reply->deleteLater();
    QJsonDocument document = ParseJsonReply(reply);
    if (document.isNull()) {
      LastRequestFailed();
      return;
    }

    QStringList list;
    QJsonArray contents = document.array();
    qLog(Debug) << "RadioBrowser station list found:" << contents.size();
    for (const QJsonValue& c : contents) {
      QJsonObject item = c.toObject();
      list << item["name"].toString();
    }

    PopulateCategory(item, list);
  });
}

void RadioBrowserService::RefreshCategoryItem(QStandardItem* item) {
  if (!EnsureServerConfig()) {
    ShowConfig();
    return;
  }

  QStandardItem* parent = item->parent();
  QString determinedUrl = parent->data(RadioBrowserService::Role_ItemsUrl)
                              .toString()
                              .arg(main_server_url_, item->text());
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  connect(reply, &QNetworkReply::finished,
          [=] { this->RefreshStreamsFinished(reply, task_id, item); });
}

void RadioBrowserService::RefreshTop100(QStandardItem* item) {
  if (!EnsureServerConfig()) {
    ShowConfig();
    return;
  }

  QString determinedUrl = item->data(RadioBrowserService::Role_ItemsUrl)
                              .toString()
                              .arg(main_server_url_);
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  connect(reply, &QNetworkReply::finished,
          [=] { this->RefreshStreamsFinished(reply, task_id, item); });
}

void RadioBrowserService::ShowContextMenu(const QPoint& global_pos) {
  if (!model()->current_index().isValid()) return;
  QStandardItem* item = model()->itemFromIndex(model()->current_index());
  if (!item) return;

  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      GetContextMenu(item)->popup(global_pos);
      break;
    case InternetModel::Type_Track:
      GetStationMenu(item)->popup(global_pos);
      break;
    default:
      break;
  }
}

QMenu* RadioBrowserService::GetContextMenu(QStandardItem* item) {
  Q_UNUSED(item);
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
    context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                             tr("Open %1 in browser").arg(homepage_url_.host()),
                             this, SLOT(Homepage()));

    context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                             tr("Refresh channels"), this,
                             SLOT(ReloadSettings()));
    context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                             tr("Configure..."), this, SLOT(ShowConfig()));
  }
  return context_menu_.get();
}

QMenu* RadioBrowserService::GetStationMenu(QStandardItem* item) {
  if (!station_menu_) {
    station_menu_.reset(new QMenu);
    station_menu_->addActions(GetPlaylistActions());
    add_to_saved_radio_action_.reset(
        new QAction(IconLoader::Load("document-open-remote", IconLoader::Base),
                    tr("Add to your radio streams"), station_menu_.get()));
    connect(add_to_saved_radio_action_.get(), &QAction::triggered, this,
            &RadioBrowserService::AddToSavedRadio);
    station_menu_->addAction(add_to_saved_radio_action_.get());
  }
  add_to_saved_radio_action_->setData(QVariant::fromValue(item));
  return station_menu_.get();
}

void RadioBrowserService::RefreshStreamsFinished(QNetworkReply* reply,
                                                 int task_id,
                                                 QStandardItem* item) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();
  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) {
    LastRequestFailed();
    return;
  }

  StreamList list;
  QJsonArray contents = document.array();
  qLog(Debug) << "RadioBrowser station list found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonObject station = c.toObject();
    ReadStation(station, &list, url_handler_);
  }

  PopulateStreams(item, list);
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

void RadioBrowserService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  main_server_url_ =
      s.value("server", QVariant(RadioBrowserService::defaultServer))
          .toString();

  if (root_ && root_->hasChildren()) {
    root_->removeRows(0, root_->rowCount());
    root_->setData(true, InternetModel::Role_CanLazyLoad);
  }
}

bool RadioBrowserService::EnsureServerConfig() {
  return !main_server_url_.isEmpty();
}

void RadioBrowserService::LastRequestFailed() { ShowConfig(); }

void RadioBrowserService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_RadioBrowser);
}

void RadioBrowserService::Search(int search_id, const QString& query,
                                 const int limit) {
  QString determinedUrl = SearchUrl.arg(main_server_url_, query).arg(limit);
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  connect(reply, &QNetworkReply::finished, [=] {
    app_->task_manager()->SetTaskFinished(task_id);
    reply->deleteLater();
    QJsonDocument document = ParseJsonReply(reply);

    StreamList list;
    QJsonArray contents = document.array();
    qLog(Debug) << "RadioBrowser station list found:" << contents.size();
    for (const QJsonValue& c : contents) {
      QJsonObject item = c.toObject();
      ReadStation(item, &list, url_handler_);
    }

    emit SearchFinished(search_id, list);
  });
}

void RadioBrowserService::ResolveStationUrl(const QUrl& original_url) {
  QString determinedUrl =
      PlayClickUrl.arg(main_server_url_, original_url.path());
  QUrl url(determinedUrl);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  int task_id = app_->task_manager()->StartTask(tr("Getting station"));

  connect(reply, &QNetworkReply::finished, [=] {
    app_->task_manager()->SetTaskFinished(task_id);
    reply->deleteLater();
    QJsonDocument document = ParseJsonReply(reply);

    QJsonObject item = document.object();
    QUrl url(item["url"].toString());
    emit StationUrlResolved(original_url, url);
  });
}

void RadioBrowserService::AddToSavedRadio(bool checked) {
  Q_UNUSED(checked);
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action) return;

  QStandardItem* item = action->data().value<QStandardItem*>();
  if (!item) return;

  Song station = item->data(InternetModel::Role_SongMetadata).value<Song>();
  InternetModel::Service<SavedRadio>()->Add(station.url(), station.artist(),
                                            QUrl(station.art_automatic()));
}