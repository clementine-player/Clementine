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

#ifndef AUTOEXPANDINGTREEVIEW_H
#define AUTOEXPANDINGTREEVIEW_H

#include <QTreeView>

class AutoExpandingTreeView : public QTreeView {
  Q_OBJECT

 public:
  AutoExpandingTreeView(QWidget* parent = nullptr);

  static const int kRowsToShow;

  void SetAutoOpen(bool v) { auto_open_ = v; }
  void SetExpandOnReset(bool v) { expand_on_reset_ = v; }
  void SetAddOnDoubleClick(bool v) { add_on_double_click_ = v; }

 public slots:
  void RecursivelyExpand(const QModelIndex& index);
  void UpAndFocus();
  void DownAndFocus();

signals:
  void AddToPlaylistSignal(QMimeData* data);
  void FocusOnFilterSignal(QKeyEvent* event);

 protected:
  // QAbstractItemView
  void reset();

  // QWidget
  void mousePressEvent(QMouseEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);

  virtual bool CanRecursivelyExpand(const QModelIndex& index) const {
    return true;
  }

 private slots:
  void ItemExpanded(const QModelIndex& index);
  void ItemClicked(const QModelIndex& index);
  void ItemDoubleClicked(const QModelIndex& index);

 private:
  bool RecursivelyExpand(const QModelIndex& index, int* count);

 private:
  bool auto_open_;
  bool expand_on_reset_;
  bool add_on_double_click_;

  bool ignore_next_click_;
};

#endif  // AUTOEXPANDINGTREEVIEW_H
