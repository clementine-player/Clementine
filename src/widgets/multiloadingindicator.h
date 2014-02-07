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

#ifndef MULTILOADINGINDICATOR_H
#define MULTILOADINGINDICATOR_H

#include <QWidget>

class BusyIndicator;
class TaskManager;

class MultiLoadingIndicator : public QWidget {
  Q_OBJECT

 public:
  MultiLoadingIndicator(QWidget* parent = 0);

  static const int kVerticalPadding;
  static const int kHorizontalPadding;
  static const int kSpacing;

  void SetTaskManager(TaskManager* task_manager);

  QSize sizeHint() const;

signals:
  void TaskCountChange(int tasks);

 protected:
  void paintEvent(QPaintEvent*);

 private slots:
  void UpdateText();

 private:
  TaskManager* task_manager_;

  BusyIndicator* spinner_;
  QString text_;
};

#endif  // MULTILOADINGINDICATOR_H
