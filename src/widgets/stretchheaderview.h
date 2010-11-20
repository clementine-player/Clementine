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

  static const float kMinimumColumnWidth;

  void setModel(QAbstractItemModel* model);

  void HideSection(int logical);
  void ShowSection(int logical);
  void SetSectionHidden(int logical, bool hidden);
  void SetColumnWidth(int logical, float width);

  bool is_stretch_enabled() const { return stretch_enabled_; }

public slots:
  void ToggleStretchEnabled();
  void SetStretchEnabled(bool enabled);

signals:
  void StretchEnabledChanged(bool enabled);

protected:
  void mouseMoveEvent(QMouseEvent* e);
  void resizeEvent(QResizeEvent* event);

private:
  void NormaliseWidths(const QList<int>& sections = QList<int>());
  void UpdateWidths(const QList<int>& sections = QList<int>());

private slots:
  void SectionResized(int logical, int old_size, int new_size);

private:
  bool stretch_enabled_;
  QVector<float> column_widths_;

  bool in_mouse_move_event_;
};

#endif // STRETCHHEADERVIEW_H
