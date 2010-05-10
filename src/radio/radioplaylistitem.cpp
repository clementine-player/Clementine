/* This file is part of Clementine.

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
    service_(service),
    url_(url),
    title_(title),
    artist_(artist)
{
  InitMetadata();
}

bool RadioPlaylistItem::InitFromQuery(const QSqlQuery &query) {
  // The song tables gets joined first, plus one each for the song ROWIDs
  const int row = (Song::kColumns.count() + 1) * 2;

  url_ = query.value(row + 1).toString();
  title_ = query.value(row + 2).toString();
  artist_ = query.value(row + 3).toString();
  QString service(query.value(row + 6).toString());

  service_ = RadioModel::ServiceByName(service);

  InitMetadata();
  return true;
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
  if (!service_)
    metadata_.set_title(QApplication::translate("RadioPlaylistItem", "Radio service couldn't be loaded :-("));
  else if (!title_.isEmpty())
    metadata_.set_title(title_);
  else
    metadata_.set_title(url_.toString());

  metadata_.set_artist(artist_);
  metadata_.set_filetype(Song::Type_Stream);

  if (service_) {
    QString icon = service_->Icon();
    if (!icon.isEmpty()) {
      metadata_.set_art_manual(icon);
    }
  }
}

Song RadioPlaylistItem::Metadata() const {
  if (temp_metadata_.is_valid())
    return temp_metadata_;
  return metadata_;
}

void RadioPlaylistItem::StartLoading() {
  if (service_)
    service_->StartLoading(url_);
}

void RadioPlaylistItem::LoadNext() {
  if (service_)
    service_->LoadNext(url_);
}

QUrl RadioPlaylistItem::Url() const {
  return url_;
}

PlaylistItem::Options RadioPlaylistItem::options() const {
  PlaylistItem::Options ret = SpecialPlayBehaviour;

  if (service_) {
    ret |= ContainsMultipleTracks;

    if (!service_->IsPauseAllowed())
      ret |= PauseDisabled;
    if (service_->ShowLastFmControls())
      ret |= LastFMControls;
  }

  return ret;
}

void RadioPlaylistItem::SetTemporaryMetadata(const Song& metadata) {
  temp_metadata_ = metadata;
  temp_metadata_.set_filetype(Song::Type_Stream);
}

void RadioPlaylistItem::ClearTemporaryMetadata() {
  temp_metadata_ = Song();
}
