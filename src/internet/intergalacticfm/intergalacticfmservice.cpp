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

#include "intergalacticfmservice.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
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
#include "globalsearch/globalsearch.h"
#include "globalsearch/intergalacticfmsearchprovider.h"
#include "intergalacticfmurlhandler.h"
#include "internet/core/internetmodel.h"
#include "ui/iconloader.h"

const int IntergalacticFMServiceBase::kStreamsCacheDurationSecs =
    60 * 60 * 24 * 28;  // 4 weeks

bool operator<(const IntergalacticFMServiceBase::Stream& a,
               const IntergalacticFMServiceBase::Stream& b) {
  return a.title_.compare(b.title_, Qt::CaseInsensitive) < 0;
}

IntergalacticFMServiceBase::IntergalacticFMServiceBase(
    Application* app, InternetModel* parent, const QString& name,
    const QUrl& channel_list_url, const QUrl& homepage_url,
    const QUrl& donate_page_url, const QIcon& icon)
    : InternetService(name, app, parent, parent),
      url_scheme_(name.toLower().remove(' ')),
      url_handler_(new IntergalacticFMUrlHandler(app, this, this)),
      root_(nullptr),
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
      new IntergalacticFMSearchProvider(this, app_, this));
}

IntergalacticFMServiceBase::~IntergalacticFMServiceBase() {}

QStandardItem* IntergalacticFMServiceBase::CreateRootItem() {
  root_ = new QStandardItem(icon_, name_);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void IntergalacticFMServiceBase::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      RefreshStreams();
      break;

    default:
      break;
  }
}

void IntergalacticFMServiceBase::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
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

void IntergalacticFMServiceBase::ForceRefreshStreams() {
  QNetworkReply* reply = network_->get(QNetworkRequest(channel_list_url_));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, &QNetworkReply::finished, this,
             &IntergalacticFMServiceBase::RefreshStreamsFinished, reply,
             task_id);
}

void IntergalacticFMServiceBase::RefreshStreamsFinished(QNetworkReply* reply,
                                                        int task_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    app_->AddError(
        tr("Failed to get channel list:\n%1").arg(reply->errorString()));
    return;
  }

  StreamList list = ParseChannels(reply->readAll());
  if (list.isEmpty()) {
    app_->AddError(tr("Failed to get channel list:\n%1")
                       .arg(tr("The server didn't list any channels")));
    return;
  }

  streams_.Update(list);
  streams_.Sort();

  // Only update the item's children if it's already been populated
  if (!root_->data(InternetModel::Role_CanLazyLoad).toBool()) PopulateStreams();

  emit StreamsChanged();
}

IntergalacticFMServiceBase::StreamList
IntergalacticFMServiceBase::ParseChannels(const QByteArray& data) const {
  // Icecast's status: a "source" for each of the server's streams, or the
  // stream itself if there's only one. Each channel has several, in different
  // formats and bitrates: take its best MP3 one.
  QJsonValue sources =
      QJsonDocument::fromJson(data).object()["icestats"].toObject()["source"];
  QJsonArray source_list =
      sources.isArray() ? sources.toArray() : QJsonArray{sources};

  QMap<QString, QJsonObject> best;
  for (const QJsonValue& value : source_list) {
    QJsonObject source = value.toObject();
    const QString name = source["server_name"].toString().trimmed();
    if (name.isEmpty() || source["server_type"].toString() != "audio/mpeg" ||
        source["listenurl"].toString().isEmpty()) {
      continue;
    }
    if (!best.contains(name) ||
        source["bitrate"].toInt() > best[name]["bitrate"].toInt()) {
      best[name] = source;
    }
  }

  StreamList ret;
  for (const QJsonObject& source : best) {
    // Its playlist, which the URL handler loads: the listen URL with ".xspf".
    // (Icecast's .m3u has no #EXTM3U header for the parser to recognise.)
    QUrl url(source["listenurl"].toString());
    url.setPort(-1);
    url.setPath(url.path() + ".xspf");
    url.setScheme(url_handler_->scheme());

    Stream stream;
    stream.title_ = source["server_name"].toString().trimmed();
    stream.url_ = url;
    ret << stream;
  }
  return ret;
}

Song IntergalacticFMServiceBase::Stream::ToSong(const QString& prefix) const {
  QString song_title = title_.trimmed();
  if (!song_title.startsWith(prefix)) {
    song_title = prefix + " " + song_title;
  }

  Song ret;
  ret.set_valid(true);
  ret.set_title(song_title);
  ret.set_artist(dj_);
  ret.set_url(url_);
  return ret;
}

void IntergalacticFMServiceBase::Homepage() {
  QDesktopServices::openUrl(homepage_url_);
}

void IntergalacticFMServiceBase::Donate() {
  QDesktopServices::openUrl(donate_page_url_);
}

PlaylistItem::Options IntergalacticFMServiceBase::playlistitem_options() const {
  return PlaylistItem::PauseDisabled;
}

IntergalacticFMServiceBase::StreamList IntergalacticFMServiceBase::Streams() {
  if (IsStreamListStale()) {
    metaObject()->invokeMethod(this, "ForceRefreshStreams",
                               Qt::QueuedConnection);
  }
  return streams_;
}

void IntergalacticFMServiceBase::RefreshStreams() {
  if (IsStreamListStale()) {
    ForceRefreshStreams();
    return;
  }
  PopulateStreams();
}

void IntergalacticFMServiceBase::PopulateStreams() {
  if (root_->hasChildren()) root_->removeRows(0, root_->rowCount());

  for (const Stream& stream : streams_) {
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("icon_radio", IconLoader::Lastfm), QString());
    item->setText(stream.title_);
    item->setData(QVariant::fromValue(stream.ToSong(name_)),
                  InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem,
                  InternetModel::Role_PlayBehaviour);

    root_->appendRow(item);
  }
}

QDataStream& operator<<(QDataStream& out,
                        const IntergalacticFMServiceBase::Stream& stream) {
  out << stream.title_ << stream.dj_ << stream.url_;
  return out;
}

QDataStream& operator>>(QDataStream& in,
                        IntergalacticFMServiceBase::Stream& stream) {
  in >> stream.title_ >> stream.dj_ >> stream.url_;
  return in;
}

void IntergalacticFMServiceBase::ReloadSettings() {
  streams_.Load();
  streams_.Sort();
}

IntergalacticFMService::IntergalacticFMService(Application* app,
                                               InternetModel* parent)
    : IntergalacticFMServiceBase(
          app, parent, "Intergalactic FM",
          // The channels.xml it used to publish is gone. Its Icecast
          // server's status lists the channels instead.
          QUrl("https://radio.intergalactic.fm/status-json.xsl"),
          QUrl("https://www.intergalactic.fm"), QUrl(),
          IconLoader::Load("intergalacticfm", IconLoader::Provider)) {}
