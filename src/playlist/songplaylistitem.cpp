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

  if (type() == "Stream") {
    QString title(query.value(row + 2).toString());
    QString artist(query.value(row + 3).toString());
    QString album(query.value(row + 4).toString());
    int length(query.value(row + 5).toInt());

    song_.set_filename(filename);
    song_.set_filetype(Song::Type_Stream);

    song_.Init(title, artist, album, length);
  } else {
    song_.InitFromFile(filename, -1);

    qint64 beginning(query.value(row + 7).toInt());
    QString cue_path(query.value(row + 8).toString());

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
  if (QFile::exists(song_.filename())) {
    return QUrl::fromLocalFile(song_.filename());
  } else {
    return song_.filename();
  }
}

void SongPlaylistItem::Reload() {
  song_.InitFromFile(song_.filename(), song_.directory_id());
}

Song SongPlaylistItem::Metadata() const {
  if (HasTemporaryMetadata())
    return temp_metadata_;
  return song_;
}


