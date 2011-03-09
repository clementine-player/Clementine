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

#include "windows7thumbbar.h"

#include <QAction>
#include <QtDebug>

#ifdef Q_OS_WIN32
# define _WIN32_WINNT 0x0600
# include <windows.h>
# include <commctrl.h>
# include <shobjidl.h>
#endif // Q_OS_WIN32


const int Windows7ThumbBar::kIconSize = 16;
const int Windows7ThumbBar::kMaxButtonCount = 7;


Windows7ThumbBar::Windows7ThumbBar(QWidget* widget)
  : QObject(widget),
    widget_(widget),
    button_created_message_id_(0),
    taskbar_list_(NULL)
{
}

void Windows7ThumbBar::SetActions(const QList<QAction*>& actions) {
#ifdef Q_OS_WIN32
  Q_ASSERT(actions.count() <= kMaxButtonCount);

  actions_ = actions;
  foreach (QAction* action, actions) {
    if (action) {
      connect(action, SIGNAL(changed()), SLOT(ActionChanged()));
    }
  }

#endif // Q_OS_WIN32
}

#ifdef Q_OS_WIN32
static void SetupButton(const QAction* action, THUMBBUTTON* button) {
  if (action) {
    button->hIcon = action->icon().pixmap(Windows7ThumbBar::kIconSize).toWinHICON();
    button->dwFlags = action->isEnabled() ? THBF_ENABLED : THBF_DISABLED;
    action->text().toWCharArray(button->szTip);
    button->szTip[action->text().count()] = '\0';

    if (!action->isVisible()) {
      button->dwFlags = THUMBBUTTONFLAGS(button->dwFlags | THBF_HIDDEN);
    }
  } else {
    button->hIcon = 0;
    button->szTip[0] = '\0';
    button->dwFlags = THBF_NOBACKGROUND;
  }
}
#endif // Q_OS_WIN32

void Windows7ThumbBar::HandleWinEvent(MSG* msg) {
#ifdef Q_OS_WIN32
  if (button_created_message_id_ == 0) {
    // Compute the value for the TaskbarButtonCreated message
    button_created_message_id_ = RegisterWindowMessage("TaskbarButtonCreated");
  }

  if (msg->message == button_created_message_id_) {
    // Unref the old taskbar list if we had one
    if (taskbar_list_) {
      reinterpret_cast<ITaskbarList3*>(taskbar_list_)->Release();
      taskbar_list_ = NULL;
    }

    if (!taskbar_list_) {
      // Create the taskbar list for the first time
      if (CoCreateInstance(CLSID_ITaskbarList, NULL, CLSCTX_ALL,
                           IID_ITaskbarList3, (void**) &taskbar_list_)) {
        qWarning() << "Error creating the ITaskbarList3 interface";
        return;
      }

      ITaskbarList3* taskbar_list = reinterpret_cast<ITaskbarList3*>(taskbar_list_);
      if (taskbar_list->HrInit()) {
        taskbar_list->Release();
        taskbar_list_ = NULL;
        return;
      }

      // Add the buttons
      THUMBBUTTON buttons[kMaxButtonCount];
      for (int i=0 ; i<actions_.count() ; ++i) {
        const QAction* action = actions_[i];
        THUMBBUTTON* button = &buttons[i];

        button->dwMask = THUMBBUTTONMASK(THB_ICON | THB_TOOLTIP | THB_FLAGS);
        button->iId = i;
        SetupButton(action, button);
      }

      taskbar_list->ThumbBarAddButtons(widget_->winId(), actions_.count(), buttons);
    }
  } else if (msg->message == WM_COMMAND) {
    const int button_id = LOWORD(msg->wParam);

    if (button_id >= 0 && button_id < actions_.count()) {
      actions_[button_id]->activate(QAction::Trigger);
    }
  }
#endif // Q_OS_WIN32
}

void Windows7ThumbBar::ActionChanged() {
#ifdef Q_OS_WIN32
  if (!taskbar_list_)
    return;
  ITaskbarList3* taskbar_list = reinterpret_cast<ITaskbarList3*>(taskbar_list_);

  THUMBBUTTON buttons[kMaxButtonCount];
  for (int i=0 ; i<actions_.count() ; ++i) {
    const QAction* action = actions_[i];
    THUMBBUTTON* button = &buttons[i];

    button->dwMask = THUMBBUTTONMASK(THB_ICON | THB_TOOLTIP | THB_FLAGS);
    button->iId = i;
    SetupButton(action, button);
  }

  taskbar_list->ThumbBarUpdateButtons(widget_->winId(), actions_.count(), buttons);
#endif // Q_OS_WIN32
}
