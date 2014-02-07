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

#include "groupediconview.h"
#include "core/multisortfilterproxy.h"

#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QtDebug>

const int GroupedIconView::kBarThickness = 2;
const int GroupedIconView::kBarMarginTop = 3;

GroupedIconView::GroupedIconView(QWidget* parent)
    : QListView(parent),
      proxy_model_(new MultiSortFilterProxy(this)),
      default_header_height_(fontMetrics().height() + kBarMarginTop +
                             kBarThickness),
      header_spacing_(10),
      header_indent_(5),
      item_indent_(10),
      header_text_("%1") {
  setFlow(LeftToRight);
  setViewMode(IconMode);
  setResizeMode(Adjust);
  setWordWrap(true);
  setDragEnabled(false);

  proxy_model_->AddSortSpec(Role_Group);
  proxy_model_->setDynamicSortFilter(true);

  connect(proxy_model_, SIGNAL(modelReset()), SLOT(LayoutItems()));
}

void GroupedIconView::AddSortSpec(int role, Qt::SortOrder order) {
  proxy_model_->AddSortSpec(role, order);
}

void GroupedIconView::setModel(QAbstractItemModel* model) {
  proxy_model_->setSourceModel(model);
  proxy_model_->sort(0);

  QListView::setModel(proxy_model_);
  LayoutItems();
}

int GroupedIconView::header_height() const { return default_header_height_; }

void GroupedIconView::DrawHeader(QPainter* painter, const QRect& rect,
                                 const QFont& font, const QPalette& palette,
                                 const QString& text) {
  painter->save();

  // Bold font
  QFont bold_font(font);
  bold_font.setBold(true);
  QFontMetrics metrics(bold_font);

  QRect text_rect(rect);
  text_rect.setHeight(metrics.height());
  text_rect.moveTop(
      rect.top() +
      (rect.height() - text_rect.height() - kBarThickness - kBarMarginTop) / 2);
  text_rect.setLeft(text_rect.left() + 3);

  // Draw text
  painter->setFont(bold_font);
  painter->drawText(text_rect, text);

  // Draw a line underneath
  const QPoint start(rect.left(), text_rect.bottom() + kBarMarginTop);
  const QPoint end(rect.right(), start.y());

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(palette.color(QPalette::Disabled, QPalette::Text),
                       kBarThickness, Qt::SolidLine, Qt::RoundCap));
  painter->setOpacity(0.5);
  painter->drawLine(start, end);

  painter->restore();
}

void GroupedIconView::resizeEvent(QResizeEvent* e) {
  QListView::resizeEvent(e);
  LayoutItems();
}

void GroupedIconView::rowsInserted(const QModelIndex& parent, int start,
                                   int end) {
  QListView::rowsInserted(parent, start, end);
  LayoutItems();
}

void GroupedIconView::dataChanged(const QModelIndex& topLeft,
                                  const QModelIndex& bottomRight) {
  QListView::dataChanged(topLeft, bottomRight);
  LayoutItems();
}

void GroupedIconView::LayoutItems() {
  if (!model()) return;

  const int count = model()->rowCount();

  QString last_group;
  QPoint next_position(0, 0);
  int max_row_height = 0;

  visual_rects_.clear();
  visual_rects_.reserve(count);
  headers_.clear();

  for (int i = 0; i < count; ++i) {
    const QModelIndex index(model()->index(i, 0));
    const QString group = index.data(Role_Group).toString();
    const QSize size(rectForIndex(index).size());

    // Is this the first item in a new group?
    if (group != last_group) {
      // Add the group header.
      Header header;
      header.y = next_position.y() + max_row_height + header_indent_;
      header.first_row = i;
      header.text = group;

      if (!last_group.isNull()) {
        header.y += header_spacing_;
      }

      headers_ << header;

      // Remember this group so we don't add it again.
      last_group = group;

      // Move the next item immediately below the header.
      next_position.setX(0);
      next_position.setY(header.y + header_height() + header_indent_ +
                         header_spacing_);
      max_row_height = 0;
    }

    // Take into account padding and spacing
    QPoint this_position(next_position);
    if (this_position.x() == 0) {
      this_position.setX(this_position.x() + item_indent_);
    } else {
      this_position.setX(this_position.x() + spacing());
    }

    // Should this item wrap?
    if (next_position.x() != 0 &&
        this_position.x() + size.width() >= viewport()->width()) {
      next_position.setX(0);
      next_position.setY(next_position.y() + max_row_height);
      this_position = next_position;
      this_position.setX(this_position.x() + item_indent_);

      max_row_height = 0;
    }

    // Set this item's geometry
    visual_rects_.append(QRect(this_position, size));

    // Update next index
    next_position.setX(this_position.x() + size.width());
    max_row_height = qMax(max_row_height, size.height());
  }

  verticalScrollBar()->setRange(
      0, next_position.y() + max_row_height - viewport()->height());
  update();
}

QRect GroupedIconView::visualRect(const QModelIndex& index) const {
  if (index.row() < 0 || index.row() >= visual_rects_.count()) return QRect();
  return visual_rects_[index.row()].translated(-horizontalOffset(),
                                               -verticalOffset());
}

QModelIndex GroupedIconView::indexAt(const QPoint& p) const {
  const QPoint viewport_p = p + QPoint(horizontalOffset(), verticalOffset());

  const int count = visual_rects_.count();
  for (int i = 0; i < count; ++i) {
    if (visual_rects_[i].contains(viewport_p)) {
      return model()->index(i, 0);
    }
  }
  return QModelIndex();
}

void GroupedIconView::paintEvent(QPaintEvent* e) {
  // This code was adapted from QListView::paintEvent(), changed to use the
  // visualRect() of items, and to draw headers.

  QStyleOptionViewItemV4 option(viewOptions());
  if (isWrapping()) option.features = QStyleOptionViewItemV2::WrapText;
  option.locale = locale();
  option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
  option.widget = this;

  QPainter painter(viewport());

  const QRect viewport_rect(
      e->rect().translated(horizontalOffset(), verticalOffset()));
  QVector<QModelIndex> toBeRendered = IntersectingItems(viewport_rect);

  const QModelIndex current = currentIndex();
  const QAbstractItemModel* itemModel = model();
  const QItemSelectionModel* selections = selectionModel();
  const bool focus =
      (hasFocus() || viewport()->hasFocus()) && current.isValid();
  const QStyle::State state = option.state;
  const QAbstractItemView::State viewState = this->state();
  const bool enabled = (state & QStyle::State_Enabled) != 0;

  int maxSize = (flow() == TopToBottom)
                    ? viewport()->size().width() - 2 * spacing()
                    : viewport()->size().height() - 2 * spacing();

  QVector<QModelIndex>::const_iterator end = toBeRendered.constEnd();
  for (QVector<QModelIndex>::const_iterator it = toBeRendered.constBegin();
       it != end; ++it) {
    if (!it->isValid()) {
      continue;
    }

    option.rect = visualRect(*it);

    if (flow() == TopToBottom)
      option.rect.setWidth(qMin(maxSize, option.rect.width()));
    else
      option.rect.setHeight(qMin(maxSize, option.rect.height()));

    option.state = state;
    if (selections && selections->isSelected(*it))
      option.state |= QStyle::State_Selected;
    if (enabled) {
      QPalette::ColorGroup cg;
      if ((itemModel->flags(*it) & Qt::ItemIsEnabled) == 0) {
        option.state &= ~QStyle::State_Enabled;
        cg = QPalette::Disabled;
      } else {
        cg = QPalette::Normal;
      }
      option.palette.setCurrentColorGroup(cg);
    }
    if (focus && current == *it) {
      option.state |= QStyle::State_HasFocus;
      if (viewState == EditingState) option.state |= QStyle::State_Editing;
    }

    itemDelegate()->paint(&painter, option, *it);
  }

  // Draw headers
  foreach(const Header & header, headers_) {
    const QRect header_rect =
        QRect(header_indent_, header.y,
              viewport()->width() - header_indent_ * 2, header_height());

    // Is this header contained in the area we're drawing?
    if (!header_rect.intersects(viewport_rect)) {
      continue;
    }

    // Draw the header
    DrawHeader(&painter,
               header_rect.translated(-horizontalOffset(), -verticalOffset()),
               font(), palette(),
               model()->index(header.first_row, 0).data(Role_Group).toString());
  }
}

void GroupedIconView::setSelection(
    const QRect& rect, QItemSelectionModel::SelectionFlags command) {
  QVector<QModelIndex> indexes(
      IntersectingItems(rect.translated(horizontalOffset(), verticalOffset())));
  QItemSelection selection;

  foreach(const QModelIndex & index, indexes) {
    selection << QItemSelectionRange(index);
  }

  selectionModel()->select(selection, command);
}

QVector<QModelIndex> GroupedIconView::IntersectingItems(const QRect& rect)
    const {
  QVector<QModelIndex> ret;

  const int count = visual_rects_.count();
  for (int i = 0; i < count; ++i) {
    if (rect.intersects(visual_rects_[i])) {
      ret.append(model()->index(i, 0));
    }
  }

  return ret;
}

QRegion GroupedIconView::visualRegionForSelection(
    const QItemSelection& selection) const {
  QRegion ret;
  foreach(const QModelIndex & index, selection.indexes()) {
    ret += visual_rects_[index.row()];
  }
  return ret;
}

QModelIndex GroupedIconView::moveCursor(CursorAction action,
                                        Qt::KeyboardModifiers) {
  if (model()->rowCount() == 0) {
    return QModelIndex();
  }

  int ret = currentIndex().row();
  if (ret == -1) {
    ret = 0;
  }

  switch (action) {
    case MoveUp:
      ret = IndexAboveOrBelow(ret, -1);
      break;
    case MovePrevious:
    case MoveLeft:
      ret--;
      break;
    case MoveDown:
      ret = IndexAboveOrBelow(ret, +1);
      break;
    case MoveNext:
    case MoveRight:
      ret++;
      break;
    case MovePageUp:
    case MoveHome:
      ret = 0;
      break;
    case MovePageDown:
    case MoveEnd:
      ret = model()->rowCount() - 1;
      break;
  }

  return model()->index(qBound(0, ret, model()->rowCount()), 0);
}

int GroupedIconView::IndexAboveOrBelow(int index, int d) const {
  const QRect orig_rect(visual_rects_[index]);

  while (index >= 0 && index < visual_rects_.count()) {
    const QRect rect(visual_rects_[index]);
    const QPoint center(rect.center());

    if ((center.y() <= orig_rect.top() || center.y() >= orig_rect.bottom()) &&
        center.x() >= orig_rect.left() && center.x() <= orig_rect.right()) {
      return index;
    }

    index += d;
  }

  return index;
}
