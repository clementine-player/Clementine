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

#include "radioplaylistitem.h"
#include "radioservice.h"
#include "radiomodel.h"
#include "core/settingsprovider.h"
#include "library/sqlrow.h"

#include <QSettings>
#include <QApplication>
#include <QtDebug>

RadioPlaylistItem::RadioPlaylistItem(const QString& type)
  : PlaylistItem(type),
    set_service_icon_(false)
{
}

RadioPlaylistItem::RadioPlaylistItem(RadioService* service, const QUrl& url,
                                     const QString& title, const QString& artist)
  : PlaylistItem("Radio"),
    url_(url),
    title_(title),
    artist_(artist),
    service_name_(service->name()),
    set_service_icon_(false)
{
  InitMetadata();
}

bool RadioPlaylistItem::InitFromQuery(const SqlRow& query) {
  // The song tables gets joined first, plus one each for the song ROWIDs
  const int row = (Song::kColumns.count() + 1) * 3;

  url_ = query.value(row + 1).toString();
  title_ = query.value(row + 2).toString();
  artist_ = query.value(row + 3).toString();
  service_name_ = query.value(row + 6).toString();

  InitMetadata();
  return true;
}

RadioService* RadioPlaylistItem::service() const {
  RadioService* ret = RadioModel::ServiceByName(service_name_);

  if (ret && !set_service_icon_) {
    const_cast<RadioPlaylistItem*>(this)->set_service_icon_ = true;

    QString icon = ret->Icon();
    if (!icon.isEmpty()) {
      const_cast<RadioPlaylistItem*>(this)->metadata_.set_art_manual(icon);
    }
  }

  return ret;
}

QVariant RadioPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_Url:          return url_.toString();
    case Column_Title:        return title_;
    case Column_Artist:       return artist_;
    case Column_RadioService: return service_name_;
    default:                  return PlaylistItem::DatabaseValue(column);
  }
}

void RadioPlaylistItem::InitMetadata() {
  if (!title_.isEmpty())
    metadata_.set_title(title_);
  else
    metadata_.set_title(url_.toString());

  metadata_.set_artist(artist_);
  metadata_.set_filetype(Song::Type_Stream);
}

Song RadioPlaylistItem::Metadata() const {
  if (!set_service_icon_) {
    // Get the icon if we don't have it already
    service();
  }

  if (HasTemporaryMetadata())
    return temp_metadata_;
  return metadata_;
}

PlaylistItem::SpecialLoadResult RadioPlaylistItem::StartLoading() {
  RadioService* s = service();
  if (!s)
    return SpecialLoadResult();
  return s->StartLoading(url_);
}

PlaylistItem::SpecialLoadResult RadioPlaylistItem::LoadNext() {
  RadioService* s = service();
  if (!s)
    return SpecialLoadResult();
  return s->LoadNext(url_);
}

QUrl RadioPlaylistItem::Url() const {
  return url_;
}

PlaylistItem::Options RadioPlaylistItem::options() const {
  RadioService* s = service();
  if (!s)
    return Default;
  return s->playlistitem_options();
}
