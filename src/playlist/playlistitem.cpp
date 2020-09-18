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

#include <QSqlQuery>
#include <QtConcurrentRun>
#include <QtDebug>

#include "core/logging.h"
#include "core/song.h"
#include "internet/core/internetplaylistitem.h"
#include "internet/jamendo/jamendoplaylistitem.h"
#include "internet/jamendo/jamendoservice.h"
#include "internet/magnatune/magnatuneplaylistitem.h"
#include "internet/magnatune/magnatuneservice.h"
#include "library/library.h"
#include "library/libraryplaylistitem.h"
#include "songplaylistitem.h"

PlaylistItem::~PlaylistItem() {}

PlaylistItem* PlaylistItem::NewFromType(const QString& type) {
  if (LibraryPlaylistItem::IsTypeSupported(type))
    return new LibraryPlaylistItem(type);
  if (MagnatunePlaylistItem::IsTypeSupported(type))
    return new MagnatunePlaylistItem(type);
  if (JamendoPlaylistItem::IsTypeSupported(type))
    return new JamendoPlaylistItem(type);
  if (SongPlaylistItem::IsTypeSupported(type))
    return new SongPlaylistItem(type);
  if (InternetPlaylistItem::IsTypeSupported(type))
    return new InternetPlaylistItem(type);

  qLog(Warning) << "Invalid PlaylistItem type:" << type;
  return nullptr;
}

PlaylistItem* PlaylistItem::NewFromSongsTable(const QString& table,
                                              const Song& song) {
  if (table == Library::kSongsTable) return new LibraryPlaylistItem(song);
  if (table == MagnatuneService::kSongsTable)
    return new MagnatunePlaylistItem(song);
  if (table == JamendoService::kSongsTable)
    return new JamendoPlaylistItem(song);

  qLog(Warning) << "Invalid PlaylistItem songs table:" << table;
  return nullptr;
}

void PlaylistItem::BindToQuery(QSqlQuery* query) const {
  query->bindValue(":type", type());
  query->bindValue(":library_id", DatabaseValue(Column_LibraryId));
  query->bindValue(":radio_service", DatabaseValue(Column_InternetService));

  DatabaseSongMetadata().BindToQuery(query);
}

void PlaylistItem::SetTemporaryMetadata(const Song& metadata) {
  temp_metadata_ = metadata;
  temp_metadata_.set_filetype(Song::Type_Stream);
}

void PlaylistItem::ClearTemporaryMetadata() { temp_metadata_ = Song(); }

static void ReloadPlaylistItem(PlaylistItemPtr item) { item->Reload(); }

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
void PlaylistItem::SetShouldSkip(bool val) { should_skip_ = val; }
bool PlaylistItem::GetShouldSkip() const { return should_skip_; }
