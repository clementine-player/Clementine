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

#ifndef GROUPEDICONVIEW_H
#define GROUPEDICONVIEW_H

#include <QListView>

class MultiSortFilterProxy;

class GroupedIconView : public QListView {
  Q_OBJECT

  // Vertical space separating a header from the items above and below it.
  Q_PROPERTY(int header_spacing READ header_spacing WRITE set_header_spacing)

  // Horizontal space separating a header from the left and right edges of the
  // widget.
  Q_PROPERTY(int header_indent READ header_indent WRITE set_header_indent)

  // Horizontal space separating an item from the left and right edges of the
  // widget.
  Q_PROPERTY(int item_indent READ item_indent WRITE set_item_indent)

  // The text of each group's header.  Must contain "%1".
  Q_PROPERTY(QString header_text READ header_text WRITE set_header_text);

 public:
  GroupedIconView(QWidget* parent = nullptr);

  enum Role { Role_Group = 1158300, };

  void AddSortSpec(int role, Qt::SortOrder order = Qt::AscendingOrder);

  int header_spacing() const { return header_spacing_; }
  int header_indent() const { return header_indent_; }
  int item_indent() const { return item_indent_; }
  const QString& header_text() const { return header_text_; }

  void set_header_spacing(int value) { header_spacing_ = value; }
  void set_header_indent(int value) { header_indent_ = value; }
  void set_item_indent(int value) { item_indent_ = value; }
  void set_header_text(const QString& value) { header_text_ = value; }

  // QAbstractItemView
  QModelIndex moveCursor(CursorAction action, Qt::KeyboardModifiers modifiers);
  void setModel(QAbstractItemModel* model);

  static void DrawHeader(QPainter* painter, const QRect& rect,
                         const QFont& font, const QPalette& palette,
                         const QString& text);

 protected:
  virtual int header_height() const;

  // QWidget
  void paintEvent(QPaintEvent* e);
  void resizeEvent(QResizeEvent* e);

  // QAbstractItemView
  void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& = QVector<int>());
  QModelIndex indexAt(const QPoint& p) const;
  void rowsInserted(const QModelIndex& parent, int start, int end);
  void setSelection(const QRect& rect,
                    QItemSelectionModel::SelectionFlags command);
  QRect visualRect(const QModelIndex& index) const;
  QRegion visualRegionForSelection(const QItemSelection& selection) const;

 private slots:
  void LayoutItems();

 private:
  static const int kBarThickness;
  static const int kBarMarginTop;

  struct Header {
    int y;
    int first_row;
    QString text;
  };

  // Returns the items that are wholly or partially inside the rect.
  QVector<QModelIndex> IntersectingItems(const QRect& rect) const;

  // Returns the index of the item above (d=-1) or below (d=+1) the given item.
  int IndexAboveOrBelow(int index, int d) const;

  MultiSortFilterProxy* proxy_model_;
  QVector<QRect> visual_rects_;
  QVector<Header> headers_;

  const int default_header_height_;
  int header_spacing_;
  int header_indent_;
  int item_indent_;
  QString header_text_;
};

#endif  // GROUPEDICONVIEW_H
