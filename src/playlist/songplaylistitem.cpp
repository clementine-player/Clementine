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

#include "songplaylistitem.h"

#include "library/sqlrow.h"

#include <QtDebug>
#include <QFile>
#include <QSettings>

SongPlaylistItem::SongPlaylistItem(const QString& type)
  : PlaylistItem(type)
{
}

SongPlaylistItem::SongPlaylistItem(const Song& song)
  : PlaylistItem(song.filetype() == Song::Type_Stream ? "Stream" : "File"),
    song_(song)
{
}

bool SongPlaylistItem::InitFromQuery(const SqlRow& query) {
  // The song tables get joined first, plus one each for the song ROWIDs
  const int row = (Song::kColumns.count() + 1) * 3;

  QString filename(query.value(row + 1).toString());
  QString title(query.value(row + 2).toString());
  QString artist(query.value(row + 3).toString());
  QString album(query.value(row + 4).toString());
  qint64 length(query.value(row + 5).toLongLong());

  if (type() == "Stream") {
    song_.set_filename(filename);
    song_.set_filetype(Song::Type_Stream);

    song_.Init(title, artist, album, length);
  } else {
    song_.InitFromFile(filename, -1);

    qint64 beginning(query.value(row + 7).toLongLong());
    QString cue_path(query.value(row + 8).toString());

    // If the song was part of a cuesheet then keep the title, artist etc. that
    // was loaded last time.
    if (!cue_path.isEmpty()) {
      song_.set_title(title);
      song_.set_artist(artist);
      song_.set_album(album);
      song_.set_length_nanosec(length);
    }

    song_.set_beginning_nanosec(beginning);
    song_.set_cue_path(cue_path);
  }

  return true;
}

QVariant SongPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_Url:       return song_.filename();
    case Column_Title:     return song_.title();
    case Column_Artist:    return song_.artist();
    case Column_Album:     return song_.album();
    case Column_Length:    return song_.length_nanosec();
    case Column_Beginning: return song_.beginning_nanosec();
    case Column_CuePath:   return song_.cue_path();
    default:               return PlaylistItem::DatabaseValue(column);
  }
}

QUrl SongPlaylistItem::Url() const {
  return song_.url();
}

void SongPlaylistItem::Reload() {
  QString old_filename = song_.filename();
  int old_directory_id = song_.directory_id();

  song_ = Song();
  song_.InitFromFile(old_filename, old_directory_id);
}

Song SongPlaylistItem::Metadata() const {
  if (HasTemporaryMetadata())
    return temp_metadata_;
  return song_;
}


