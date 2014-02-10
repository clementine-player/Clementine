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

#include "albumcovermanagerlist.h"

#include <memory>

#include <QDropEvent>
#include <QUrl>

#include "albumcovermanager.h"
#include "library/librarybackend.h"
#include "playlist/songmimedata.h"

AlbumCoverManagerList::AlbumCoverManagerList(QWidget* parent)
    : QListWidget(parent), manager_(nullptr) {}

QMimeData* AlbumCoverManagerList::mimeData(const QList<QListWidgetItem*> items)
    const {
  // Get songs
  SongList songs;
  for (QListWidgetItem* item : items) {
    songs << manager_->GetSongsInAlbum(indexFromItem(item));
  }

  if (songs.isEmpty()) return nullptr;

  // Get URLs from the songs
  QList<QUrl> urls;
  for (const Song& song : songs) {
    urls << song.url();
  }

  // Get the QAbstractItemModel data so the picture works
  std::unique_ptr<QMimeData> orig_data(QListWidget::mimeData(items));

  SongMimeData* mime_data = new SongMimeData;
  mime_data->backend = manager_->backend();
  mime_data->songs = songs;
  mime_data->setUrls(urls);
  mime_data->setData(orig_data->formats()[0],
                     orig_data->data(orig_data->formats()[0]));
  return mime_data;
}

void AlbumCoverManagerList::dropEvent(QDropEvent* e) {
  // Set movement to Static just for this dropEvent so the user can't move the
  // album covers.  If it's set to Static all the time then the user can't even
  // drag to the playlist
  QListWidget::Movement old_movement = movement();
  setMovement(QListWidget::Static);
  QListWidget::dropEvent(e);
  setMovement(old_movement);
}
