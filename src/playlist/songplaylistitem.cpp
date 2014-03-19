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

#include "playlistbackend.h"
#include "songplaylistitem.h"
#include "core/tagreaderclient.h"

#include "library/sqlrow.h"

#include <QtDebug>
#include <QFile>
#include <QSettings>

SongPlaylistItem::SongPlaylistItem(const QString& type) : PlaylistItem(type) {}

SongPlaylistItem::SongPlaylistItem(const Song& song)
    : PlaylistItem(song.is_stream() ? "Stream" : "File"), song_(song) {}

bool SongPlaylistItem::InitFromQuery(const SqlRow& query) {
  song_.InitFromQuery(query, false, (Song::kColumns.count() + 1) * 3);

  if (type() == "Stream") {
    song_.set_filetype(Song::Type_Stream);
  }

  return true;
}

QUrl SongPlaylistItem::Url() const { return song_.url(); }

void SongPlaylistItem::Reload() {
  if (song_.url().scheme() != "file") return;

  TagReaderClient::Instance()->ReadFileBlocking(song_.url().toLocalFile(),
                                                &song_);
}

Song SongPlaylistItem::Metadata() const {
  if (HasTemporaryMetadata()) return temp_metadata_;
  return song_;
}
