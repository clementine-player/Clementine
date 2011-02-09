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

#include "radioservice.h"
#include "radiomodel.h"
#include "core/mergedproxymodel.h"
#include "core/mimedata.h"

RadioService::RadioService(const QString& name, RadioModel* model)
  : QObject(model),
    model_(model),
    name_(name)
{
}

PlaylistItem::SpecialLoadResult RadioService::StartLoading(const QUrl &url) {
  return PlaylistItem::SpecialLoadResult(
      PlaylistItem::SpecialLoadResult::TrackAvailable, url, url);
}

PlaylistItem::SpecialLoadResult RadioService::LoadNext(const QUrl&) {
  return PlaylistItem::SpecialLoadResult();
}

void RadioService::AddItemToPlaylist(const QModelIndex& index, AddMode add_mode) {
  AddItemsToPlaylist(QModelIndexList() << index, add_mode);
}

void RadioService::AddItemsToPlaylist(const QModelIndexList& indexes, AddMode add_mode) {
  QMimeData* data = model()->merged_model()->mimeData(
        model()->merged_model()->mapFromSource(indexes));
  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    mime_data->clear_first_ = add_mode == AddMode_Replace;
    mime_data->new_playlist_ = add_mode == AddMode_OpenInNew;
  }
  emit AddToPlaylistSignal(data);
}
