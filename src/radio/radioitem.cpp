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

#include "radioitem.h"
#include "radioservice.h"

RadioItem::RadioItem(SimpleTreeModel<RadioItem> *model)
  : SimpleTreeItem<RadioItem>(Type_Root, model),
    service(NULL),
    use_song_loader(false)
{
}

RadioItem::RadioItem(RadioService* _service, int type, const QString& key,
                     RadioItem* parent)
  : SimpleTreeItem<RadioItem>(type, key, parent),
    service(_service),
    playable(false),
    use_song_loader(false)
{
}

QUrl RadioItem::Url() const {
  return service->UrlForItem(this);
}

QString RadioItem::Title() const {
  return service->TitleForItem(this);
}

QString RadioItem::Artist() const {
  return service->ArtistForItem(this);
}
