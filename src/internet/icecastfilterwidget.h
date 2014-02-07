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

#ifndef ICECASTFILTERWIDGET_H
#define ICECASTFILTERWIDGET_H

#include "icecastmodel.h"

#include <QWidget>

class LineEditInterface;
class Ui_IcecastFilterWidget;

class QActionGroup;
class QMenu;
class QSignalMapper;

class IcecastFilterWidget : public QWidget {
  Q_OBJECT

 public:
  IcecastFilterWidget(QWidget* parent = 0);
  ~IcecastFilterWidget();

  static const char* kSettingsGroup;

  void SetIcecastModel(IcecastModel* model);

  QMenu* menu() const { return menu_; }

 public slots:
  void FocusOnFilter(QKeyEvent* e);

 private slots:
  void SortModeChanged(int mode);

 private:
  void AddAction(QActionGroup* group, QAction* action,
                 IcecastModel::SortMode mode);

 private:
  Ui_IcecastFilterWidget* ui_;
  IcecastModel* model_;
  QMenu* menu_;

  QSignalMapper* sort_mode_mapper_;
};

#endif  // ICECASTFILTERWIDGET_H
