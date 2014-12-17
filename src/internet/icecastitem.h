/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_ICECASTITEM_H_
#define INTERNET_ICECASTITEM_H_

#include "icecastbackend.h"
#include "core/simpletreeitem.h"

class IcecastItem : public SimpleTreeItem<IcecastItem> {
 public:
  enum Type {
    Type_Root,
    Type_Genre,
    Type_Station,
    Type_Divider,
  };

  explicit IcecastItem(SimpleTreeModel<IcecastItem>* model)
    : SimpleTreeItem<IcecastItem>(Type_Root, model) {}
  explicit IcecastItem(Type type, IcecastItem* parent = nullptr)
    : SimpleTreeItem<IcecastItem>(type, parent) {}

  IcecastBackend::Station station;
};

#endif  // INTERNET_ICECASTITEM_H_
