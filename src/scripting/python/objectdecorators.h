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

#ifndef OBJECTDECORATORS_H
#define OBJECTDECORATORS_H

#include <QObject>
#include <QSqlQuery>
#include <QUrl>

#include "core/song.h"
#include "playlist/playlistitem.h"

class ObjectDecorators : public QObject {
  Q_OBJECT

public slots:
  // QObject
  void deleteLater(QObject* self);
  bool inherits(QObject* self, const char* class_name);

  // PlaylistItemPtr
  QString type(PlaylistItemPtr* self) const;
  PlaylistItem::Options options(PlaylistItemPtr* self) const;
  bool InitFromQuery(PlaylistItemPtr* self, const SqlRow& query);
  void BindToQuery(PlaylistItemPtr* self, QSqlQuery* query) const;
  void Reload(PlaylistItemPtr* self);
  void BackgroundReload(PlaylistItemPtr* self);
  Song Metadata(PlaylistItemPtr* self) const;
  QUrl Url(PlaylistItemPtr* self) const;
  void SetTemporaryMetadata(PlaylistItemPtr* self, const Song& metadata);
  void ClearTemporaryMetadata(PlaylistItemPtr* self);
  bool HasTemporaryMetadata(PlaylistItemPtr* self) const;
  void SetBackgroundColor(PlaylistItemPtr* self, short priority, const QColor& color);
  bool HasBackgroundColor(PlaylistItemPtr* self, short priority) const;
  void RemoveBackgroundColor(PlaylistItemPtr* self, short priority);
  QColor GetCurrentBackgroundColor(PlaylistItemPtr* self) const;
  bool HasCurrentBackgroundColor(PlaylistItemPtr* self) const;
  void SetForegroundColor(PlaylistItemPtr* self, short priority, const QColor& color);
  bool HasForegroundColor(PlaylistItemPtr* self, short priority) const;
  void RemoveForegroundColor(PlaylistItemPtr* self, short priority);
  QColor GetCurrentForegroundColor(PlaylistItemPtr* self) const;
  bool HasCurrentForegroundColor(PlaylistItemPtr* self) const;
  bool IsLocalLibraryItem(PlaylistItemPtr* self) const;
};

#endif // OBJECTDECORATORS_H
