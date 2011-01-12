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
    service_(NULL)
{
}

RadioPlaylistItem::RadioPlaylistItem(RadioService* service, const QUrl& url,
                                     const QString& title, const QString& artist)
  : PlaylistItem("Radio"),
    url_(url),
    title_(title),
    artist_(artist),
    service_name_(service->name()),
    service_(service)
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

  // It's ok if this returns NULL at this point - the service might get added
  // later eg. by a plugin.
  service_ = RadioModel::ServiceByName(service_name_);

  InitMetadata();
  return true;
}

RadioService* RadioPlaylistItem::service() const {
  if (!service_) {
    // Lazy load the service if it's not cached
    const_cast<RadioPlaylistItem*>(this)->service_ =
        RadioModel::ServiceByName(service_name_);

    // Maybe we should set an icon now as well
    if (service_) {
      QString icon = service_->Icon();
      if (!icon.isEmpty()) {
        const_cast<RadioPlaylistItem*>(this)->metadata_.set_art_manual(icon);
      }
    }
  }

  return service_;
}

QVariant RadioPlaylistItem::DatabaseValue(DatabaseColumn column) const {
  switch (column) {
    case Column_Url:          return url_.toString();
    case Column_Title:        return title_;
    case Column_Artist:       return artist_;
    case Column_RadioService: return service_->name();
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

  if (service()) {
    QString icon = service_->Icon();
    if (!icon.isEmpty()) {
      metadata_.set_art_manual(icon);
    }
  }
}

Song RadioPlaylistItem::Metadata() const {
  if (HasTemporaryMetadata())
    return temp_metadata_;
  return metadata_;
}

PlaylistItem::SpecialLoadResult RadioPlaylistItem::StartLoading() {
  if (!service())
    return SpecialLoadResult();
  return service_->StartLoading(url_);
}

PlaylistItem::SpecialLoadResult RadioPlaylistItem::LoadNext() {
  if (!service())
    return SpecialLoadResult();
  return service_->LoadNext(url_);
}

QUrl RadioPlaylistItem::Url() const {
  return url_;
}

PlaylistItem::Options RadioPlaylistItem::options() const {
  if (!service())
    return Default;
  return service_->playlistitem_options();
}
