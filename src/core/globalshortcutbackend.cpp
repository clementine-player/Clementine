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

#include "globalshortcutbackend.h"
#include "globalshortcuts.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

GlobalShortcutBackend::GlobalShortcutBackend(GlobalShortcuts *parent)
  : QObject(parent),
    manager_(parent),
    active_(false),
    register_in_progress_(false),
    should_unregister_(false)
{
}

void GlobalShortcutBackend::Register() {
  if (register_in_progress_) {
    should_unregister_ = false;
    return;
  }

  if (RegisterInNewThread()) {
    register_in_progress_ = true;
    QFuture<bool> future = QtConcurrent::run(this, &GlobalShortcutBackend::DoRegister);

    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), SLOT(RegisterFinishedSlot()));
  } else {
    bool ret = DoRegister();
    if (ret)
      active_ = true;
    emit RegisterFinished(ret);
  }
}

void GlobalShortcutBackend::Unregister() {
  if (register_in_progress_) {
    should_unregister_ = true;
    return;
  }

  DoUnregister();
  active_ = false;
}

void GlobalShortcutBackend::RegisterFinishedSlot() {
  QFutureWatcher<bool>* watcher = dynamic_cast<QFutureWatcher<bool>*>(sender());
  const bool success = watcher->result();
  watcher->deleteLater();

  register_in_progress_ = false;
  if (success)
    active_ = true;

  if (should_unregister_) {
    Unregister();
    should_unregister_ = false;
  } else {
    emit RegisterFinished(success);
  }
}
