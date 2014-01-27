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

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QMap>
#include <QMetaType>
#include <QStandardItem>
#include <QUrl>

#include <boost/enable_shared_from_this.hpp>

#include "core/song.h"

class QAction;
class SqlRow;

class PlaylistItem : public boost::enable_shared_from_this<PlaylistItem> {
 public:
  PlaylistItem(const QString& type)
    : should_skip_(false),
      type_(type) {}
  virtual ~PlaylistItem();

  static PlaylistItem* NewFromType(const QString& type);
  static PlaylistItem* NewFromSongsTable(const QString& table, const Song& song);

  enum Option {
    Default = 0x00,

    // Disables the "pause" action.
    PauseDisabled = 0x01,

    // Enables the last.fm "ban" action.
    LastFMControls = 0x02,

    // Disables the seek slider.
    SeekDisabled = 0x04,
  };
  Q_DECLARE_FLAGS(Options, Option);

  virtual QString type() const { return type_; }

  virtual Options options() const { return Default; }

  virtual QList<QAction*> actions() { return QList<QAction*>(); }

  virtual bool InitFromQuery(const SqlRow& query) = 0;
  void BindToQuery(QSqlQuery* query) const;
  virtual void Reload() {}
  QFuture<void> BackgroundReload();

  virtual Song Metadata() const = 0;
  virtual QUrl Url() const = 0;

  void SetTemporaryMetadata(const Song& metadata);
  void ClearTemporaryMetadata();
  bool HasTemporaryMetadata() const { return temp_metadata_.is_valid(); }

  // Background colors.
  void SetBackgroundColor(short priority, const QColor& color);
  bool HasBackgroundColor(short priority) const;
  void RemoveBackgroundColor(short priority);
  QColor GetCurrentBackgroundColor() const;
  bool HasCurrentBackgroundColor() const;

  // Foreground colors.
  void SetForegroundColor(short priority, const QColor& color);
  bool HasForegroundColor(short priority) const;
  void RemoveForegroundColor(short priority);
  QColor GetCurrentForegroundColor() const;
  bool HasCurrentForegroundColor() const;

  // Convenience function to find out whether this item is from the local
  // library, as opposed to a device, a file on disk, or a stream.
  // Remember that even if this returns true, the library item might be
  // invalid so you might want to check that its id is not equal to -1
  // before actually using it.
  virtual bool IsLocalLibraryItem() const { return false; }
  void SetShouldSkip(bool val);
  bool GetShouldSkip() const;

 protected:
  bool should_skip_;

  enum DatabaseColumn {
    Column_LibraryId,
    Column_InternetService,
  };

  virtual QVariant DatabaseValue(DatabaseColumn) const {
    return QVariant(QVariant::String); }
  virtual Song DatabaseSongMetadata() const { return Song(); }

  QString type_;

  Song temp_metadata_;

  QMap<short, QColor> background_colors_;
  QMap<short, QColor> foreground_colors_;
};
typedef boost::shared_ptr<PlaylistItem> PlaylistItemPtr;
typedef QList<PlaylistItemPtr> PlaylistItemList;

Q_DECLARE_METATYPE(PlaylistItemPtr)
Q_DECLARE_METATYPE(QList<PlaylistItemPtr>)
Q_DECLARE_OPERATORS_FOR_FLAGS(PlaylistItem::Options)

#endif // PLAYLISTITEM_H
