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
#include "radiomodel.h"
#include "core/network.h"
#include "core/taskmanager.h"
#include "ui/iconloader.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QSettings>
#include <QTemporaryFile>
#include <QMenu>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QtDebug>

const char* SomaFMService::kServiceName = "SomaFM";
const char* SomaFMService::kChannelListUrl = "http://somafm.com/channels.xml";
const char* SomaFMService::kHomepage = "http://somafm.com";

SomaFMService::SomaFMService(RadioModel* parent)
  : RadioService(kServiceName, parent),
    root_(NULL),
    context_menu_(NULL),
    get_channels_task_id_(0),
    get_stream_task_id_(0),
    network_(new NetworkAccessManager(this))
{
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
    context_menu_->addAction(IconLoader::Load("media-playback-start"), tr("Add to playlist"), this, SLOT(AddToPlaylist()));
    context_menu_->addAction(IconLoader::Load("media-playback-start"), tr("Load"), this, SLOT(LoadToPlaylist()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download"), tr("Open somafm.com in browser"), this, SLOT(Homepage()));
    context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Refresh channels"), this, SLOT(RefreshChannels()));
  }

  context_item_ = model()->itemFromIndex(index);
  context_menu_->popup(global_pos);
}

PlaylistItem::SpecialLoadResult SomaFMService::StartLoading(const QUrl& url) {
  // Load the playlist
  QNetworkRequest request = QNetworkRequest(url);
  request.setRawHeader("User-Agent", QString("%1 %2").arg(
      QCoreApplication::applicationName(), QCoreApplication::applicationVersion()).toUtf8());

  QNetworkReply* reply = network_->get(request);
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));

  if (!get_stream_task_id_)
    get_stream_task_id_ = model()->task_manager()->StartTask(tr("Loading stream"));

  return PlaylistItem::SpecialLoadResult(
      PlaylistItem::SpecialLoadResult::WillLoadAsynchronously, url);
}

void SomaFMService::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  model()->task_manager()->SetTaskFinished(get_stream_task_id_);
  get_stream_task_id_ = 0;

  QUrl original_url(reply->url());

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qDebug() << reply->errorString();
    emit AsyncLoadFinished(PlaylistItem::SpecialLoadResult(
        PlaylistItem::SpecialLoadResult::NoMoreTracks, original_url));
    return;
  }

  // TODO: Replace with some more robust .pls parsing :(
  QTemporaryFile temp_file;
  temp_file.open();
  temp_file.write(reply->readAll());
  temp_file.flush();

  QSettings s(temp_file.fileName(), QSettings::IniFormat);
  s.beginGroup("playlist");

  emit AsyncLoadFinished(PlaylistItem::SpecialLoadResult(
      PlaylistItem::SpecialLoadResult::TrackAvailable,
      original_url, s.value("File1").toString()));
}

void SomaFMService::RefreshChannels() {
  QNetworkRequest request = QNetworkRequest(QUrl(kChannelListUrl));
  request.setRawHeader("User-Agent", QString("%1 %2").arg(
      QCoreApplication::applicationName(), QCoreApplication::applicationVersion()).toUtf8());

  QNetworkReply* reply = network_->get(request);
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
    qDebug() << reply->errorString();
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
  QStandardItem* item = new QStandardItem(QIcon(":last.fm/icon_radio.png"), QString());
  item->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::EndElement:
        if (item->data(RadioModel::Role_Url).toString().isNull()) {
          // Didn't find a URL
          delete item;
        } else {
          root_->appendRow(item);
        }
        return;

      case QXmlStreamReader::StartElement:
        if (reader.name() == "title") {
          item->setText(reader.readElementText());
          item->setData("SomaFM " + item->text(), RadioModel::Role_Title);
        } else if (reader.name() == "dj") {
          item->setData(reader.readElementText(), RadioModel::Role_Artist);
        } else if (reader.name() == "fastpls" && reader.attributes().value("format") == "mp3") {
          item->setData(reader.readElementText(), RadioModel::Role_Url);
        } else {
          ConsumeElement(reader);
        }
        break;

      default:
        break;
    }
  }

  delete item;
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

void SomaFMService::AddToPlaylist() {
  AddItemToPlaylist(context_item_->index(), false);
}

void SomaFMService::LoadToPlaylist() {
  AddItemToPlaylist(context_item_->index(), true);
}

PlaylistItem::Options SomaFMService::playlistitem_options() const {
  return PlaylistItem::SpecialPlayBehaviour |
         PlaylistItem::PauseDisabled;
}
