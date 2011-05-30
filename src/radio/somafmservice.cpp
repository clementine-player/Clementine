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
#include "radiomodel.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QtDebug>

const char* SomaFMService::kServiceName = "SomaFM";
const char* SomaFMService::kChannelListUrl = "http://somafm.com/channels.xml";
const char* SomaFMService::kHomepage = "http://somafm.com";

SomaFMService::SomaFMService(RadioModel* parent)
  : RadioService(kServiceName, parent, parent),
    url_handler_(new SomaFMUrlHandler(this, this)),
    root_(NULL),
    context_menu_(NULL),
    get_channels_task_id_(0),
    network_(new NetworkAccessManager(this))
{
  model()->player()->RegisterUrlHandler(url_handler_);
}

SomaFMService::~SomaFMService() {
  delete context_menu_;
}

QStandardItem* SomaFMService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":/providers/somafm.png"), kServiceName);
  root_->setData(true, RadioModel::Role_CanLazyLoad);
  return root_;
}

void SomaFMService::LazyPopulate(QStandardItem* item) {
  switch (item->data(RadioModel::Role_Type).toInt()) {
    case RadioModel::Type_Service:
      RefreshChannels();
      break;

    default:
      break;
  }
}

void SomaFMService::ShowContextMenu(const QModelIndex& index, const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download"), tr("Open somafm.com in browser"), this, SLOT(Homepage()));
    context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Refresh channels"), this, SLOT(RefreshChannels()));
  }

  context_item_ = model()->itemFromIndex(index);
  context_menu_->popup(global_pos);
}

void SomaFMService::RefreshChannels() {
  QNetworkReply* reply = network_->get(QNetworkRequest(QUrl(kChannelListUrl)));
  connect(reply, SIGNAL(finished()), SLOT(RefreshChannelsFinished()));

  if (!get_channels_task_id_)
    get_channels_task_id_ = model()->task_manager()->StartTask(tr("Getting channels"));
}

void SomaFMService::RefreshChannelsFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  model()->task_manager()->SetTaskFinished(get_channels_task_id_);
  get_channels_task_id_ = 0;

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qLog(Error) << reply->errorString();
    return;
  }

  if (root_->hasChildren())
    root_->removeRows(0, root_->rowCount());

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "channel") {
      ReadChannel(reader);
    }
  }
}

void SomaFMService::ReadChannel(QXmlStreamReader& reader) {
  Song song;

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::EndElement:
        if (!song.url().isEmpty()) {
          QStandardItem* item = new QStandardItem(QIcon(":last.fm/icon_radio.png"), QString());
          item->setText(song.title());

          song.set_title("SomaFM " + song.title());
          item->setData(QVariant::fromValue(song), RadioModel::Role_SongMetadata);
          item->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);

          root_->appendRow(item);
        }
        return;

      case QXmlStreamReader::StartElement:
        if (reader.name() == "title") {
          song.set_title(reader.readElementText());
        } else if (reader.name() == "dj") {
          song.set_artist(reader.readElementText());
        } else if (reader.name() == "fastpls" && reader.attributes().value("format") == "mp3") {
          QUrl url(reader.readElementText());
          url.setScheme("somafm");

          song.set_url(url);
        } else {
          ConsumeElement(reader);
        }
        break;

      default:
        break;
    }
  }
}

void SomaFMService::ConsumeElement(QXmlStreamReader& reader) {
  int level = 1;
  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::StartElement: level++; break;
      case QXmlStreamReader::EndElement:   level--; break;
      default: break;
    }

    if (level == 0)
      return;
  }
}

void SomaFMService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

QModelIndex SomaFMService::GetCurrentIndex() {
  return context_item_->index();
}

PlaylistItem::Options SomaFMService::playlistitem_options() const {
  return PlaylistItem::PauseDisabled;
}
