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

#ifndef WINDOWS7THUMBBAR_H
#define WINDOWS7THUMBBAR_H

#include <QMainWindow>
#include <QWidget>

#ifndef Q_OS_WIN32
typedef void MSG;
#endif  // Q_OS_WIN32

class Windows7ThumbBar : public QObject {
  Q_OBJECT

 public:
  // Creates a list of buttons in the taskbar icon for this window.  Does
  // nothing and is safe to use on other operating systems too.
  Windows7ThumbBar(QWidget* widget = 0);

  static const int kIconSize;
  static const int kMaxButtonCount;

  // You must call this in the parent widget's constructor before returning
  // to the event loop.  If an action is NULL it becomes a spacer.
  void SetActions(const QList<QAction*>& actions);

  // Call this from the parent's winEvent() function.
  void HandleWinEvent(MSG* msg);

 private slots:
  void ActionChanged();

 private:
  QWidget* widget_;
  QList<QAction*> actions_;

  unsigned int button_created_message_id_;

  // Really an ITaskbarList3* but I don't want to have to include windows.h here
  void* taskbar_list_;
};

#endif  // WINDOWS7THUMBBAR_H
