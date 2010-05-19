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

#include "radioservice.h"
#include "radiomodel.h"

RadioService::RadioService(const QString& name, RadioModel* model)
  : QObject(model),
    model_(model),
    name_(name)
{
}

QUrl RadioService::UrlForItem(const RadioItem* item) const {
  return item->key;
}

QString RadioService::TitleForItem(const RadioItem* item) const {
  return item->DisplayText();
}

QString RadioService::ArtistForItem(const RadioItem* item) const {
  return item->artist;
}

PlaylistItem::SpecialLoadResult RadioService::StartLoading(const QUrl &url) {
  return PlaylistItem::SpecialLoadResult(
      PlaylistItem::SpecialLoadResult::TrackAvailable, url, url);
}

PlaylistItem::SpecialLoadResult RadioService::LoadNext(const QUrl&) {
  return PlaylistItem::SpecialLoadResult();
}
