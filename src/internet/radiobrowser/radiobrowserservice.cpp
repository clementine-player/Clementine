/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
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
#include "radiobrowserurlhandler.h"
#include "internet/core/internetmodel.h"
#include "ui/iconloader.h"

const int RadioBrowserServiceBase::kStreamsCacheDurationSecs =
    60 * 60 * 24 * 28;  // 4 weeks

bool operator<(const RadioBrowserServiceBase::Stream& a,
               const RadioBrowserServiceBase::Stream& b) {
  return a.name_.compare(b.name_, Qt::CaseInsensitive) < 0;
}

RadioBrowserServiceBase::RadioBrowserServiceBase(
    Application* app, InternetModel* parent, const QString& name,
    const QUrl& channel_list_url, const QUrl& homepage_url,
    const QUrl& donate_page_url, const QIcon& icon)
    : InternetService(name, app, parent, parent),
      url_scheme_(name.toLower().remove(' ')),
      url_handler_(new RadioBrowserUrlHandler(app, this, this)),
      root_(nullptr),
      context_menu_(nullptr),
      network_(new NetworkAccessManager(this)),
      streams_(name, "streams", kStreamsCacheDurationSecs),
      name_(name),
      channel_list_url_(channel_list_url),
      homepage_url_(homepage_url),
      donate_page_url_(donate_page_url),
      icon_(icon) {
  ReloadSettings();

  app_->player()->RegisterUrlHandler(url_handler_);
  app_->global_search()->AddProvider(
      new RadioBrowserSearchProvider(this, app_, this));
}

RadioBrowserServiceBase::~RadioBrowserServiceBase() {
  delete context_menu_;
}

QStandardItem* RadioBrowserServiceBase::CreateRootItem() {
  root_ = new QStandardItem(icon_, name_);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void RadioBrowserServiceBase::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      RefreshStreams();
      break;

    default:
      break;
  }
}

void RadioBrowserServiceBase::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                             tr("Open %1 in browser").arg(homepage_url_.host()),
                             this, SLOT(Homepage()));

    if (!donate_page_url_.isEmpty()) {
      context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                               tr("Donate"), this, SLOT(Donate()));
    }

    context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                             tr("Refresh channels"), this,
                             SLOT(ForceRefreshStreams()));
  }

  context_menu_->popup(global_pos);
}

void RadioBrowserServiceBase::ForceRefreshStreams() {
  QNetworkReply* reply = network_->get(QNetworkRequest(channel_list_url_));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshStreamsFinished(QNetworkReply*, int)), reply, task_id);
}

void RadioBrowserServiceBase::RefreshStreamsFinished(QNetworkReply* reply,
                                                        int task_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    app_->AddError(
        tr("Failed to get channel list:\n%1").arg(reply->errorString()));
    return;
  }

  StreamList list;

  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    app_->AddError(
        tr("Failed to parse channel list:\n%1").arg(error.errorString()));
    return;
  }

  QJsonArray contents = document.array();
  qLog(Debug) << "RadioBrowser station list found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonObject item = c.toObject();
    ReadStation(item, &list);
  }

  streams_.Update(list);
  streams_.Sort();

  // Only update the item's children if it's already been populated
  if (!root_->data(InternetModel::Role_CanLazyLoad).toBool()) PopulateStreams();

  emit StreamsChanged();
}

void RadioBrowserServiceBase::ReadStation(QJsonObject& item,
                                             StreamList* ret) {
  Stream stream;
  stream.name_ = item["name"].toString();
  QUrl url(item["url"].toString());
  stream.url_ = url;
  ret->append(stream);
}

Song RadioBrowserServiceBase::Stream::ToSong(const QString& prefix) const {
  QString song_title = name_.trimmed();
  if (!song_title.startsWith(prefix)) {
    song_title = prefix + " " + song_title;
  }

  Song ret;
  ret.set_valid(true);
  ret.set_title(song_title);
  ret.set_artist(name_);
  ret.set_url(url_);
  return ret;
}

void RadioBrowserServiceBase::Homepage() {
  QDesktopServices::openUrl(homepage_url_);
}

void RadioBrowserServiceBase::Donate() {
  QDesktopServices::openUrl(donate_page_url_);
}

PlaylistItem::Options RadioBrowserServiceBase::playlistitem_options() const {
  return PlaylistItem::PauseDisabled;
}

RadioBrowserServiceBase::StreamList RadioBrowserServiceBase::Streams() {
  if (IsStreamListStale()) {
    metaObject()->invokeMethod(this, "ForceRefreshStreams",
                               Qt::QueuedConnection);
  }
  return streams_;
}

void RadioBrowserServiceBase::RefreshStreams() {
  if (IsStreamListStale()) {
    ForceRefreshStreams();
    return;
  }
  PopulateStreams();
}

void RadioBrowserServiceBase::PopulateStreams() {
  if (root_->hasChildren()) root_->removeRows(0, root_->rowCount());

  for (const Stream& stream : streams_) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(stream.name_);
    item->setData(QVariant::fromValue(stream.ToSong(name_)),
                  InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem,
                  InternetModel::Role_PlayBehaviour);

    root_->appendRow(item);
  }
}

QDataStream& operator<<(QDataStream& out,
                        const RadioBrowserServiceBase::Stream& stream) {
  out << stream.name_ << stream.url_;
  return out;
}

QDataStream& operator>>(QDataStream& in,
                        RadioBrowserServiceBase::Stream& stream) {
  in >> stream.name_ >> stream.url_;
  return in;
}

void RadioBrowserServiceBase::ReloadSettings() {
  streams_.Load();
  streams_.Sort();
}

RadioBrowserService::RadioBrowserService(Application* app,
                                               InternetModel* parent)
    : RadioBrowserServiceBase(
          app, parent, "Radio-Browser.info",
          QUrl("http://all.api.radio-browser.info/json/stations"),
          QUrl("https://www.radio-browser.info"), QUrl(),
          IconLoader::Load("radiobrowser", IconLoader::Provider)) {}
