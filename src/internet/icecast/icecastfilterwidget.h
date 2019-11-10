/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_ICECAST_ICECASTFILTERWIDGET_H_
#define INTERNET_ICECAST_ICECASTFILTERWIDGET_H_

#include "icecastmodel.h"

#include <QWidget>

class LineEditInterface;
class Ui_IcecastFilterWidget;

class QActionGroup;
class QMenu;

class IcecastFilterWidget : public QWidget {
  Q_OBJECT

 public:
  explicit IcecastFilterWidget(QWidget* parent = nullptr);
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
};

#endif  // INTERNET_ICECAST_ICECASTFILTERWIDGET_H_
