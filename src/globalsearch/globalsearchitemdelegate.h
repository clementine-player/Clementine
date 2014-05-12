/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef GLOBALSEARCHITEMDELEGATE_H
#define GLOBALSEARCHITEMDELEGATE_H

#include "library/libraryview.h"

class GlobalSearchView;

class GlobalSearchItemDelegate : public LibraryItemDelegate {
 public:
  GlobalSearchItemDelegate(GlobalSearchView* view);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;

 private:
  GlobalSearchView* view_;
};

#endif  // GLOBALSEARCHITEMDELEGATE_H
