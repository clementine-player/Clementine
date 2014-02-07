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

#include "internetservice.h"
#include "internetmodel.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/mimedata.h"
#include "ui/iconloader.h"

#include <QMenu>
#include <QStandardItem>

InternetService::InternetService(const QString& name, Application* app,
                                 InternetModel* model, QObject* parent)
    : QObject(parent),
      app_(app),
      model_(model),
      name_(name),
      append_to_playlist_(nullptr),
      replace_playlist_(nullptr),
      open_in_new_playlist_(nullptr),
      separator_(nullptr) {}

QList<QAction*> InternetService::GetPlaylistActions() {
  if (!separator_) {
    separator_ = new QAction(this);
    separator_->setSeparator(true);
  }

  return QList<QAction*>() << GetAppendToPlaylistAction()
                           << GetReplacePlaylistAction()
                           << GetOpenInNewPlaylistAction() << separator_;
}

QAction* InternetService::GetAppendToPlaylistAction() {
  if (!append_to_playlist_) {
    append_to_playlist_ = new QAction(IconLoader::Load("media-playback-start"),
                                      tr("Append to current playlist"), this);
    connect(append_to_playlist_, SIGNAL(triggered()), this,
            SLOT(AppendToPlaylist()));
  }

  return append_to_playlist_;
}

QAction* InternetService::GetReplacePlaylistAction() {
  if (!replace_playlist_) {
    replace_playlist_ = new QAction(IconLoader::Load("media-playback-start"),
                                    tr("Replace current playlist"), this);
    connect(replace_playlist_, SIGNAL(triggered()), this,
            SLOT(ReplacePlaylist()));
  }

  return replace_playlist_;
}

QAction* InternetService::GetOpenInNewPlaylistAction() {
  if (!open_in_new_playlist_) {
    open_in_new_playlist_ = new QAction(IconLoader::Load("document-new"),
                                        tr("Open in new playlist"), this);
    connect(open_in_new_playlist_, SIGNAL(triggered()), this,
            SLOT(OpenInNewPlaylist()));
  }

  return open_in_new_playlist_;
}

void InternetService::AddItemToPlaylist(const QModelIndex& index,
                                        AddMode add_mode) {
  AddItemsToPlaylist(QModelIndexList() << index, add_mode);
}

void InternetService::AddItemsToPlaylist(const QModelIndexList& indexes,
                                         AddMode add_mode) {
  QMimeData* data = model()->merged_model()->mimeData(
      model()->merged_model()->mapFromSource(indexes));
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->clear_first_ = add_mode == AddMode_Replace;
    mime_data->open_in_new_playlist_ = add_mode == AddMode_OpenInNew;
  }
  emit AddToPlaylistSignal(data);
}

void InternetService::AppendToPlaylist() {
  AddItemsToPlaylist(model()->selected_indexes(), AddMode_Append);
}

void InternetService::ReplacePlaylist() {
  AddItemsToPlaylist(model()->selected_indexes(), AddMode_Replace);
}

void InternetService::OpenInNewPlaylist() {
  AddItemsToPlaylist(model()->selected_indexes(), AddMode_OpenInNew);
}

QStandardItem* InternetService::CreateSongItem(const Song& song) {
  QStandardItem* item = new QStandardItem(song.PrettyTitleWithArtist());
  item->setData(InternetModel::Type_Track, InternetModel::Role_Type);
  item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
  item->setData(InternetModel::PlayBehaviour_SingleItem,
                InternetModel::Role_PlayBehaviour);
  item->setData(song.url(), InternetModel::Role_Url);

  return item;
}
