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

#include "radioview.h"
#include "radiomodel.h"
#include "core/mergedproxymodel.h"
#include "library/libraryview.h"

#include <QContextMenuEvent>

RadioView::RadioView(QWidget *parent)
  : AutoExpandingTreeView(parent)
{
  setItemDelegate(new LibraryItemDelegate(this));
  SetExpandOnReset(false);
  setAttribute(Qt::WA_MacShowFocusRect, false);
}

void RadioView::contextMenuEvent(QContextMenuEvent* e) {
  QModelIndex index = indexAt(e->pos());
  if (!index.isValid())
    return;

  MergedProxyModel* merged_model = static_cast<MergedProxyModel*>(model());
  RadioModel* radio_model = static_cast<RadioModel*>(merged_model->sourceModel());

  radio_model->ShowContextMenu(index, e->globalPos());
}

void RadioView::currentChanged(const QModelIndex &current, const QModelIndex&) {
  emit CurrentIndexChanged(current);
}

void RadioView::setModel(QAbstractItemModel *model) {
  AutoExpandingTreeView::setModel(model);

  if (MergedProxyModel* merged_model = qobject_cast<MergedProxyModel*>(model)) {
    connect(merged_model,
            SIGNAL(SubModelReset(QModelIndex,QAbstractItemModel*)),
            SLOT(RecursivelyExpand(QModelIndex)));
  }
}
