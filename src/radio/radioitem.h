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

#ifndef RADIOITEM_H
#define RADIOITEM_H

#include <QIcon>
#include <QUrl>

#include "core/simpletreeitem.h"

class RadioService;

class RadioItem : public SimpleTreeItem<RadioItem> {
 public:
  enum Type {
    Type_Root = 1,
    Type_Service,
  };

  RadioItem(SimpleTreeModel<RadioItem>* model);
  RadioItem(RadioService* _service, int type, const QString& key = QString::null,
            RadioItem* parent = NULL);

  QUrl Url() const;
  QString Title() const;
  QString Artist() const;

  QIcon icon;
  QString artist;
  RadioService* service;
  bool playable;
  bool use_song_loader;
};

#endif // RADIOITEM_H
