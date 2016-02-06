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

#include "autoexpandingtreeview.h"
#include "core/mimedata.h"

#include <QMouseEvent>
#include <QtDebug>

const int AutoExpandingTreeView::kRowsToShow = 50;

AutoExpandingTreeView::AutoExpandingTreeView(QWidget* parent)
    : QTreeView(parent),
      auto_open_(true),
      expand_on_reset_(true),
      add_on_double_click_(true),
      ignore_next_click_(false) {
  setExpandsOnDoubleClick(false);
  setAnimated(true);

  connect(this, SIGNAL(expanded(QModelIndex)), SLOT(ItemExpanded(QModelIndex)));
  connect(this, SIGNAL(clicked(QModelIndex)), SLOT(ItemClicked(QModelIndex)));
  connect(this, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(ItemDoubleClicked(QModelIndex)));
}

void AutoExpandingTreeView::reset() {
  QTreeView::reset();

  // Expand nodes in the tree until we have about 50 rows visible in the view
  if (auto_open_ && expand_on_reset_) {
    RecursivelyExpand(rootIndex());
  }
}

void AutoExpandingTreeView::RecursivelyExpand(const QModelIndex& index) {
  int rows = model()->rowCount(index);
  RecursivelyExpand(index, &rows);
}

bool AutoExpandingTreeView::RecursivelyExpand(const QModelIndex& index,
                                              int* count) {
  if (!CanRecursivelyExpand(index)) return true;

  if (model()->canFetchMore(index)) model()->fetchMore(index);

  int children = model()->rowCount(index);
  if (*count + children > kRowsToShow) return false;

  expand(index);
  *count += children;

  for (int i = 0; i < children; ++i) {
    if (!RecursivelyExpand(model()->index(i, 0, index), count)) return false;
  }

  return true;
}

void AutoExpandingTreeView::ItemExpanded(const QModelIndex& index) {
  if (model()->rowCount(index) == 1 && auto_open_)
    expand(model()->index(0, 0, index));
}

void AutoExpandingTreeView::ItemClicked(const QModelIndex& index) {
  if (ignore_next_click_) {
    ignore_next_click_ = false;
    return;
  }

  setExpanded(index, !isExpanded(index));
}

void AutoExpandingTreeView::ItemDoubleClicked(const QModelIndex& index) {
  ignore_next_click_ = true;

  if (add_on_double_click_) {
    QMimeData* data = model()->mimeData(QModelIndexList() << index);
    if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
      mime_data->from_doubleclick_ = true;
    }
    emit AddToPlaylistSignal(data);
  }
}

void AutoExpandingTreeView::mousePressEvent(QMouseEvent* event) {
  if (event->modifiers() != Qt::NoModifier) {
    ignore_next_click_ = true;
  }

  QTreeView::mousePressEvent(event);

  // enqueue to playlist with middleClick
  if (event->button() == Qt::MidButton) {
    QMimeData* data = model()->mimeData(selectedIndexes());
    if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
      mime_data->enqueue_now_ = true;
    }
    emit AddToPlaylistSignal(data);
  }
}

void AutoExpandingTreeView::mouseDoubleClickEvent(QMouseEvent* event) {
  State p_state = state();
  QModelIndex index = indexAt(event->pos());

  QTreeView::mouseDoubleClickEvent(event);

  // If the p_state was the "AnimatingState", then the base class's
  // "mouseDoubleClickEvent" method just did nothing, hence the
  // "doubleClicked" signal is not emitted. So let's do it ourselves.
  if (index.isValid() && p_state == AnimatingState) {
    emit doubleClicked(index);
  }
}

void AutoExpandingTreeView::keyPressEvent(QKeyEvent* e) {
  QModelIndex index = currentIndex();

  switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if (index.isValid()) emit doubleClicked(index);
      e->accept();
      break;

    case Qt::Key_Backspace:
    case Qt::Key_Escape:
      emit FocusOnFilterSignal(e);
      e->accept();
      break;

    case Qt::Key_Left:
      // Set focus on the root of the current branch
      if (index.isValid() && index.parent() != rootIndex() &&
          (!isExpanded(index) || model()->rowCount(index) == 0)) {
        setCurrentIndex(index.parent());
        setFocus();
        e->accept();
      }
      break;
  }

  QTreeView::keyPressEvent(e);
}

void AutoExpandingTreeView::UpAndFocus() {
  setCurrentIndex(moveCursor(QAbstractItemView::MoveUp, Qt::NoModifier));
  setFocus();
}

void AutoExpandingTreeView::DownAndFocus() {
  setCurrentIndex(moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier));
  setFocus();
}
