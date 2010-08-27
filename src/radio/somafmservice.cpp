/* This file is part of Clementine.

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
#include "core/networkaccessmanager.h"
#include "core/taskmanager.h"
#include "ui/iconloader.h"

#include <QNetworkAccessManager>
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
    network_(parent->network()->network())
{
}

SomaFMService::~SomaFMService() {
  delete context_menu_;
}

RadioItem* SomaFMService::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, kServiceName, parent);
  root_->icon = QIcon(":somafm.png");
  return root_;
}

void SomaFMService::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      RefreshChannels();
      break;

    default:
      break;
  }

  item->lazy_loaded = true;
}

void SomaFMService::ShowContextMenu(RadioItem* item, const QModelIndex&,
                                    const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addAction(IconLoader::Load("media-playback-start"), tr("Add to playlist"), this, SLOT(AddToPlaylist()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download"), tr("Open somafm.com in browser"), this, SLOT(Homepage()));
    context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Refresh channels"), this, SLOT(RefreshChannels()));
  }

  context_item_ = item;
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

  root_->ClearNotify();

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "channel") {
      ReadChannel(reader);
    }
  }

  root_->lazy_loaded = true;
}

void SomaFMService::ReadChannel(QXmlStreamReader& reader) {
  RadioItem* item = new RadioItem(this, Type_Stream, QString::null);
  item->lazy_loaded = true;
  item->playable = true;
  item->icon = QIcon(":last.fm/icon_radio.png");

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::EndElement:
        if (item->key.isNull()) {
          // Didn't find a URL
          delete item;
        } else {
          item->InsertNotify(root_);
        }
        return;

      case QXmlStreamReader::StartElement:
        if (reader.name() == "title") {
          item->display_text = reader.readElementText();
        } else if (reader.name() == "dj") {
          item->artist = reader.readElementText();
        } else if (reader.name() == "fastpls" && reader.attributes().value("format") == "mp3") {
          item->key = reader.readElementText();
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

QString SomaFMService::TitleForItem(const RadioItem* item) const {
  return "SomaFM " + item->display_text;
}

void SomaFMService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

void SomaFMService::AddToPlaylist() {
  emit AddItemToPlaylist(context_item_);
}

PlaylistItem::Options SomaFMService::playlistitem_options() const {
  return PlaylistItem::SpecialPlayBehaviour |
         PlaylistItem::PauseDisabled;
}
