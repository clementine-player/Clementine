/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "eventcallbacktable.h"

#include <QMetaObject>
#include <QMutexLocker>
#include <QObject>

void EventCallbackTable::Set(const QByteArray& token, const QHostAddress& peer,
                             QObject* receiver) {
  QMutexLocker l(&mutex_);
  entries_[token] = Entry{peer, receiver};
}

void EventCallbackTable::Remove(const QByteArray& token) {
  QMutexLocker l(&mutex_);
  entries_.remove(token);
}

bool EventCallbackTable::Find(const QByteArray& token,
                              QHostAddress* peer) const {
  QMutexLocker l(&mutex_);
  auto it = entries_.constFind(token);
  if (it == entries_.constEnd()) return false;
  *peer = it->peer;
  return true;
}

bool EventCallbackTable::Deliver(const QByteArray& token,
                                 const QByteArray& body) const {
  // Held while posting, so that the receiver, which removes itself first,
  // can't be deleted meanwhile. Qt drops what's posted to it once it is.
  QMutexLocker l(&mutex_);
  auto it = entries_.constFind(token);
  if (it == entries_.constEnd()) return false;
  return QMetaObject::invokeMethod(it->receiver, "EventReceived",
                                   Qt::QueuedConnection,
                                   Q_ARG(QByteArray, body));
}
