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
#include "settingsprovider.h"

#include <QSettings>
#include <QApplication>

RadioPlaylistItem::RadioPlaylistItem()
  : service_(NULL)
{
}

RadioPlaylistItem::RadioPlaylistItem(RadioService* service, const QUrl& url,
                                     const QString& title, const QString& artist)
  : service_(service),
    url_(url),
    title_(title),
    artist_(artist)
{
  InitMetadata();
}

void RadioPlaylistItem::Save(SettingsProvider& settings) const {
  settings.setValue("service", service_->name());
  settings.setValue("url", url_.toString());
  settings.setValue("title", title_);
  settings.setValue("artist", artist_);
}

void RadioPlaylistItem::Restore(const SettingsProvider& settings) {
  service_ = RadioModel::ServiceByName(settings.value("service").toString());
  url_ = settings.value("url").toString();
  title_ = settings.value("title").toString();
  artist_ = settings.value("artist").toString();

  InitMetadata();
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
