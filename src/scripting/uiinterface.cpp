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

#include "uiinterface.h"

#include <QAction>
#include <QMenu>
#include <QtDebug>

UIInterface::UIInterface(QObject* parent)
  : QObject(parent)
{
}

void UIInterface::RegisterActionLocation(const QString& id, QMenu* menu, QAction* before) {
  if (locations_.contains(id)) {
    qDebug() << __PRETTY_FUNCTION__
             << "A location with ID" << id << "was already registered";
    return;
  }

  locations_[id] = Location(menu, before);

  connect(menu, SIGNAL(destroyed()), SLOT(MenuDestroyed()));
  if (before) {
    connect(before, SIGNAL(destroyed()), SLOT(MenuActionDestroyed()));
  }

  // Add any actions that were waiting
  foreach (const IdAndAction& id_action, pending_actions_) {
    if (id_action.first == id) {
      DoAddAction(id_action.first, id_action.second);
      pending_actions_.removeAll(id_action);
    }
  }
}

void UIInterface::AddAction(const QString& id, QAction* action) {
  if (locations_.contains(id)) {
    DoAddAction(id, action);
  } else {
    // Maybe that part of the UI hasn't been lazy created yet
    pending_actions_ << IdAndAction(id, action);
    connect(action, SIGNAL(destroyed()), SLOT(ActionDestroyed()));
  }
}

void UIInterface::DoAddAction(const QString& id, QAction* action) {
  const Location& location = locations_[id];

  if (location.menu_) {
    location.menu_->insertAction(location.before_, action);
  }
}

void UIInterface::MenuDestroyed() {
  QMenu* menu = qobject_cast<QMenu*>(sender());
  foreach (const QString& id, locations_.keys()) {
    if (locations_[id].menu_ == menu) {
      locations_.remove(id);
    }
  }
}

void UIInterface::MenuActionDestroyed() {
  QAction* action = qobject_cast<QAction*>(sender());
  foreach (const QString& id, locations_.keys()) {
    if (locations_[id].before_ == action) {
      locations_.remove(id);
    }
  }
}

void UIInterface::ActionDestroyed() {
  QAction* action = qobject_cast<QAction*>(sender());
  foreach (const IdAndAction& id_action, pending_actions_) {
    if (id_action.second == action) {
      pending_actions_.removeAll(id_action);
    }
  }
}
