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

#ifndef STRETCHHEADERVIEW_H
#define STRETCHHEADERVIEW_H

#include <QHeaderView>

class StretchHeaderView : public QHeaderView {
  Q_OBJECT

public:
  StretchHeaderView(Qt::Orientation orientation, QWidget* parent = 0);

  typedef double ColumnWidthType;

  static const int kMinimumColumnWidth;
  static const int kMagicNumber;

  void setModel(QAbstractItemModel* model);

  // Serialises the proportional and actual column widths.  Use these instead
  // of QHeaderView::restoreState and QHeaderView::saveState to persist the
  // proportional values directly and avoid floating point errors over time.
  bool RestoreState(const QByteArray& data);
  QByteArray SaveState() const;

  // Hides a section and resizes all other sections to fill the gap.  Does
  // nothing if you try to hide the last section.
  void HideSection(int logical);

  // Shows a section and resizes all other sections to make room.
  void ShowSection(int logical);

  // Calls either HideSection or ShowSection.
  void SetSectionHidden(int logical, bool hidden);

  // Sets the width of the given column and resizes other columns appropriately.
  // width is the proportion of the entire width from 0.0 to 1.0.
  void SetColumnWidth(int logical, ColumnWidthType width);

  bool is_stretch_enabled() const { return stretch_enabled_; }

public slots:
  // Changes the stretch mode.  Enabling stretch mode will initialise the
  // proportional column widths from the current state of the header.
  void ToggleStretchEnabled();
  void SetStretchEnabled(bool enabled);

signals:
  // Emitted when the stretch mode is changed.
  void StretchEnabledChanged(bool enabled);

protected:
  // QWidget
  void mouseMoveEvent(QMouseEvent* e);
  void resizeEvent(QResizeEvent* event);

private:
  // Scales column_widths_ values so the total is 1.0.
  void NormaliseWidths(const QList<int>& sections = QList<int>());

  // Resizes the actual columns to make them match the proportional values
  // in column_widths_.
  void UpdateWidths(const QList<int>& sections = QList<int>());

private slots:
  void SectionResized(int logical, int old_size, int new_size);

private:
  bool stretch_enabled_;
  QVector<ColumnWidthType> column_widths_;

  bool in_mouse_move_event_;
};

#endif // STRETCHHEADERVIEW_H
