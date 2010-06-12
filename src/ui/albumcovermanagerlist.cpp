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

#include "albumcovermanager.h"
#include "albumcovermanagerlist.h"
#include "playlist/songmimedata.h"

#include <boost/scoped_ptr.hpp>

#include <QUrl>
#include <QtDebug>

AlbumCoverManagerList::AlbumCoverManagerList(QWidget *parent)
  : QListWidget(parent),
    manager_(NULL)
{
}

QMimeData* AlbumCoverManagerList::mimeData(const QList<QListWidgetItem*> items) const {
  // Get songs
  SongList songs;
  foreach (QListWidgetItem* item, items) {
    songs << manager_->GetSongsInAlbum(indexFromItem(item));
  }

  if (songs.isEmpty())
    return NULL;

  // Get URLs from the songs
  QList<QUrl> urls;
  foreach (const Song& song, songs) {
    urls << QUrl::fromLocalFile(song.filename());
  }

  // Get the QAbstractItemModel data so the picture works
  boost::scoped_ptr<QMimeData> orig_data(QListWidget::mimeData(items));

  SongMimeData* mime_data = new SongMimeData;
  mime_data->backend = manager_->backend();
  mime_data->songs = songs;
  mime_data->setUrls(urls);
  mime_data->setData(orig_data->formats()[0], orig_data->data(orig_data->formats()[0]));
  return mime_data;
}
