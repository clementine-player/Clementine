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

#include "scriptdialogtabwidget.h"

#include <QTabBar>

ScriptDialogTabWidget::ScriptDialogTabWidget(QWidget* parent)
  : QTabWidget(parent)
{
  connect(this, SIGNAL(currentChanged(int)), SLOT(CurrentChanged(int)));
}

void ScriptDialogTabWidget::SetTabAlert(int index) {
  if (currentIndex() == index)
    return;

  tabs_with_alerts_.insert(index);
  tabBar()->setTabTextColor(index, Qt::red);
}

void ScriptDialogTabWidget::CurrentChanged(int index) {
  if (tabs_with_alerts_.contains(index)) {
    tabs_with_alerts_.remove(index);
    tabBar()->setTabTextColor(index, palette().color(QPalette::Text));
  }
}
