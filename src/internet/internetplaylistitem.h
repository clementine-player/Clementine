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

#ifndef INTERNETPLAYLISTITEM_H
#define INTERNETPLAYLISTITEM_H

#include "core/song.h"
#include "playlist/playlistitem.h"

#include <QUrl>

class InternetService;

class InternetPlaylistItem : public PlaylistItem {
 public:
  InternetPlaylistItem(const QString& type);
  InternetPlaylistItem(InternetService* service, const Song& metadata);

  Options options() const;

  QList<QAction*> actions();

  bool InitFromQuery(const SqlRow& query);

  Song Metadata() const;
  QUrl Url() const;

 protected:
  QVariant DatabaseValue(DatabaseColumn) const;
  Song DatabaseSongMetadata() const { return metadata_; }

 private:
  void InitMetadata();
  InternetService* service() const;

 private:
  QString service_name_;

  bool set_service_icon_;

  Song metadata_;
};

#endif  // INTERNETPLAYLISTITEM_H
