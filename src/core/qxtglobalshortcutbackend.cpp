/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
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

#include "globalshortcuts.h"
#include "qxtglobalshortcutbackend.h"
#include "qxtglobalshortcut.h"
#include "core/logging.h"

#include <QAction>
#include <QtDebug>

QxtGlobalShortcutBackend::QxtGlobalShortcutBackend(GlobalShortcuts* parent)
    : GlobalShortcutBackend(parent) {}

bool QxtGlobalShortcutBackend::DoRegister() {
  qLog(Debug) << "registering";
  for (const GlobalShortcuts::Shortcut& shortcut :
       manager_->shortcuts().values()) {
    AddShortcut(shortcut.action);
  }

  return true;
}

void QxtGlobalShortcutBackend::AddShortcut(QAction* action) {
  if (action->shortcut().isEmpty()) return;
  QxtGlobalShortcut* shortcut = new QxtGlobalShortcut(action->shortcut(), this);
  connect(shortcut, SIGNAL(activated()), action, SLOT(trigger()));
  shortcuts_ << shortcut;
}

void QxtGlobalShortcutBackend::DoUnregister() {
  qLog(Debug) << "unregistering";
  qDeleteAll(shortcuts_);
  shortcuts_.clear();
}
