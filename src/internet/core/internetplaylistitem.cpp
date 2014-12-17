/* This file is part of Clementine.
   Copyright 2009-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "internetplaylistitem.h"
#include "internetservice.h"
#include "internetmodel.h"
#include "core/settingsprovider.h"
#include "library/sqlrow.h"
#include "playlist/playlistbackend.h"

#include <QSettings>
#include <QApplication>
#include <QtDebug>

InternetPlaylistItem::InternetPlaylistItem(const QString& type)
    : PlaylistItem(type), set_service_icon_(false) {}

InternetPlaylistItem::InternetPlaylistItem(InternetService* service,
                                           const Song& metadata)
    : PlaylistItem("Internet"),
      service_name_(service->name()),
      set_service_icon_(false),
      metadata_(metadata) {
  InitMetadata();
}

bool InternetPlaylistItem::InitFromQuery(const SqlRow& query) {
  // The song tables gets joined first, plus one each for the song ROWIDs
  const int row =
      (Song::kColumns.count() + 1) * PlaylistBackend::kSongTableJoins;

  service_name_ = query.value(row + 1).toString();

  metadata_.InitFromQuery(query, false, (Song::kColumns.count() + 1) * 3);
  InitMetadata();

  return true;
}

InternetService* InternetPlaylistItem::service() const {
  InternetService* ret = InternetModel::ServiceByName(service_name_);

  if (ret && !set_service_icon_) {
    const_cast<InternetPlaylistItem*>(this)->set_service_icon_ = true;

    QString icon = ret->Icon();
    if (!icon.isEmpty()) {
      const_cast<InternetPlaylistItem*>(this)->metadata_.set_art_manual(icon);
    }
  }

  return ret;
}

QVariant InternetPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_InternetService:
      return service_name_;
    default:
      return PlaylistItem::DatabaseValue(column);
  }
}

void InternetPlaylistItem::InitMetadata() {
  if (metadata_.title().isEmpty())
    metadata_.set_title(metadata_.url().toString());
  metadata_.set_filetype(Song::Type_Stream);
  metadata_.set_valid(true);
}

Song InternetPlaylistItem::Metadata() const {
  if (!set_service_icon_) {
    // Get the icon if we don't have it already
    service();
  }

  if (HasTemporaryMetadata()) return temp_metadata_;
  return metadata_;
}

QUrl InternetPlaylistItem::Url() const { return metadata_.url(); }

PlaylistItem::Options InternetPlaylistItem::options() const {
  InternetService* s = service();
  if (!s) return Default;
  return s->playlistitem_options();
}

QList<QAction*> InternetPlaylistItem::actions() {
  InternetService* s = service();
  if (!s) return QList<QAction*>();
  return s->playlistitem_actions(metadata_);
}
