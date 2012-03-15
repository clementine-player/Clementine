/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "somafmservice.h"
#include "somafmurlhandler.h"
#include "internetmodel.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/somafmsearchprovider.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QtDebug>

const char* SomaFMService::kServiceName = "SomaFM";
const char* SomaFMService::kSettingsGroup = "SomaFM";
const char* SomaFMService::kChannelListUrl = "http://somafm.com/channels.xml";
const char* SomaFMService::kHomepage = "http://somafm.com";
const int SomaFMService::kStreamsCacheDurationSecs =
    60 * 60 * 24 * 28; // 4 weeks

bool operator <(const SomaFMService::Stream& a,
                const SomaFMService::Stream& b) {
  return a.title_.compare(b.title_, Qt::CaseInsensitive) < 0;
}

SomaFMService::SomaFMService(Application* app, InternetModel* parent)
  : InternetService(kServiceName, app, parent, parent),
    url_handler_(new SomaFMUrlHandler(app, this, this)),
    root_(NULL),
    context_menu_(NULL),
    network_(new NetworkAccessManager(this)),
    streams_(kSettingsGroup, "streams", kStreamsCacheDurationSecs)
{
  ReloadSettings();

  app_->player()->RegisterUrlHandler(url_handler_);
  app_->global_search()->AddProvider(new SomaFMSearchProvider(this, app_, this));
}

SomaFMService::~SomaFMService() {
  delete context_menu_;
}

QStandardItem* SomaFMService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":/providers/somafm.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SomaFMService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      RefreshStreams();
      break;

    default:
      break;
  }
}

void SomaFMService::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download"), tr("Open %1 in browser").arg("somafm.com"), this, SLOT(Homepage()));
    context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Refresh channels"), this, SLOT(RefreshStreams()));
  }

  context_menu_->popup(global_pos);
}

void SomaFMService::ForceRefreshStreams() {
  QNetworkReply* reply = network_->get(QNetworkRequest(QUrl(kChannelListUrl)));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RefreshStreamsFinished(QNetworkReply*,int)),
             reply, task_id);
}

void SomaFMService::RefreshStreamsFinished(QNetworkReply* reply, int task_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qLog(Error) << reply->errorString();
    return;
  }

  StreamList list;

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "channel") {
      ReadChannel(reader, &list);
    }
  }

  streams_.Update(list);
  streams_.Sort();

  // Only update the item's children if it's already been populated
  if (!root_->data(InternetModel::Role_CanLazyLoad).toBool())
    PopulateStreams();

  emit StreamsChanged();
}

void SomaFMService::ReadChannel(QXmlStreamReader& reader, StreamList* ret) {
  Stream stream;

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::EndElement:
        if (!stream.url_.isEmpty()) {
          ret->append(stream);
        }
        return;

      case QXmlStreamReader::StartElement:
        if (reader.name() == "title") {
          stream.title_ = reader.readElementText();
        } else if (reader.name() == "dj") {
          stream.dj_ = reader.readElementText();
        } else if (reader.name() == "fastpls" && reader.attributes().value("format") == "mp3") {
          QUrl url(reader.readElementText());
          url.setScheme("somafm");

          stream.url_ = url;
        } else {
          Utilities::ConsumeCurrentElement(&reader);
        }
        break;

      default:
        break;
    }
  }
}

Song SomaFMService::Stream::ToSong() const {
  Song ret;
  ret.set_valid(true);
  ret.set_title("SomaFM " + title_);
  ret.set_artist(dj_);
  ret.set_url(url_);
  return ret;
}

void SomaFMService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

PlaylistItem::Options SomaFMService::playlistitem_options() const {
  return PlaylistItem::PauseDisabled;
}

SomaFMService::StreamList SomaFMService::Streams() {
  if (IsStreamListStale()) {
    metaObject()->invokeMethod(this, "ForceRefreshStreams", Qt::QueuedConnection);
  }
  return streams_;
}

void SomaFMService::RefreshStreams() {
  if (IsStreamListStale()) {
    ForceRefreshStreams();
    return;
  }
  PopulateStreams();
}

void SomaFMService::PopulateStreams() {
  if (root_->hasChildren())
    root_->removeRows(0, root_->rowCount());

  foreach (const Stream& stream, streams_) {
    QStandardItem* item = new QStandardItem(QIcon(":last.fm/icon_radio.png"), QString());
    item->setText(stream.title_);
    item->setData(QVariant::fromValue(stream.ToSong()), InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);

    root_->appendRow(item);
  }
}

QDataStream& operator<<(QDataStream& out, const SomaFMService::Stream& stream) {
  out << stream.title_
      << stream.dj_
      << stream.url_;
  return out;
}

QDataStream& operator>>(QDataStream& in, SomaFMService::Stream& stream) {
  in >> stream.title_
     >> stream.dj_
     >> stream.url_;
  return in;
}

void SomaFMService::ReloadSettings() {
  streams_.Load();
  streams_.Sort();
}
