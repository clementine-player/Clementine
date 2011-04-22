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

#include "playlistitem.h"
#include "songplaylistitem.h"
#include "core/logging.h"
#include "library/library.h"
#include "library/libraryplaylistitem.h"
#include "radio/jamendoplaylistitem.h"
#include "radio/jamendoservice.h"
#include "radio/magnatuneplaylistitem.h"
#include "radio/magnatuneservice.h"
#include "radio/radioplaylistitem.h"

#include <QtConcurrentRun>
#include <QtDebug>

PlaylistItem::SpecialLoadResult::SpecialLoadResult(
    Type type, const QUrl& original_url, const QUrl& media_url)
      : type_(type), original_url_(original_url), media_url_(media_url)
{
}

PlaylistItem* PlaylistItem::NewFromType(const QString& type) {
  if (type == "Library")
    return new LibraryPlaylistItem(type);
  if (type == "Magnatune")
    return new MagnatunePlaylistItem(type);
  if (type == "Jamendo")
    return new JamendoPlaylistItem(type);
  if (type == "Stream" || type == "File")
    return new SongPlaylistItem(type);
  if (type == "Radio")
    return new RadioPlaylistItem(type);

  qLog(Warning) << "Invalid PlaylistItem type:" << type;
  return NULL;
}

PlaylistItem* PlaylistItem::NewFromSongsTable(const QString& table, const Song& song) {
  if (table == Library::kSongsTable)
    return new LibraryPlaylistItem(song);
  if (table == MagnatuneService::kSongsTable)
    return new MagnatunePlaylistItem(song);
  if (table == JamendoService::kSongsTable)
    return new JamendoPlaylistItem(song);

  qLog(Warning) << "Invalid PlaylistItem songs table:" << table;
  return NULL;
}

void PlaylistItem::BindToQuery(QSqlQuery* query) const {
  query->bindValue(1, type());
  query->bindValue(2, DatabaseValue(Column_LibraryId));
  query->bindValue(3, DatabaseValue(Column_Url));
  query->bindValue(4, DatabaseValue(Column_Title));
  query->bindValue(5, DatabaseValue(Column_Artist));
  query->bindValue(6, DatabaseValue(Column_Album));
  query->bindValue(7, DatabaseValue(Column_Length));
  query->bindValue(8, DatabaseValue(Column_RadioService));
  query->bindValue(9, DatabaseValue(Column_Beginning));
  query->bindValue(10, DatabaseValue(Column_CuePath));
}

void PlaylistItem::SetTemporaryMetadata(const Song& metadata) {
  temp_metadata_ = metadata;
  temp_metadata_.set_filetype(Song::Type_Stream);
}

void PlaylistItem::ClearTemporaryMetadata() {
  temp_metadata_ = Song();
}

static void ReloadPlaylistItem(PlaylistItemPtr item) {
  item->Reload();
}

QFuture<void> PlaylistItem::BackgroundReload() {
  return QtConcurrent::run(ReloadPlaylistItem, shared_from_this());
}

void PlaylistItem::SetBackgroundColor(short priority, const QColor& color) {
  background_colors_[priority] = color;
}
bool PlaylistItem::HasBackgroundColor(short priority) const {
  return background_colors_.contains(priority);
}
void PlaylistItem::RemoveBackgroundColor(short priority) {
  background_colors_.remove(priority);
}
QColor PlaylistItem::GetCurrentBackgroundColor() const {
  return background_colors_.isEmpty()
      ? QColor()
      : background_colors_[background_colors_.keys().last()];
}
bool PlaylistItem::HasCurrentBackgroundColor() const {
  return !background_colors_.isEmpty();
}

void PlaylistItem::SetForegroundColor(short priority, const QColor& color) {
  foreground_colors_[priority] = color;
}
bool PlaylistItem::HasForegroundColor(short priority) const {
  return foreground_colors_.contains(priority);
}
void PlaylistItem::RemoveForegroundColor(short priority) {
  foreground_colors_.remove(priority);
}
QColor PlaylistItem::GetCurrentForegroundColor() const {
  return foreground_colors_.isEmpty()
      ? QColor()
      : foreground_colors_[foreground_colors_.keys().last()];
}
bool PlaylistItem::HasCurrentForegroundColor() const {
  return !foreground_colors_.isEmpty();
}
