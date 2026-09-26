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

#include "streamitemtable.h"

#include <QMutexLocker>

void StreamItemTable::Set(const QByteArray& token, const QHostAddress& peer,
                          const QList<StreamItem>& items) {
  QMutexLocker l(&mutex_);
  entries_[token] = Entry{peer, items};
}

void StreamItemTable::Remove(const QByteArray& token) {
  QMutexLocker l(&mutex_);
  entries_.remove(token);
}

bool StreamItemTable::Find(const QByteArray& token, int id, StreamItem* item,
                           QHostAddress* peer) const {
  QMutexLocker l(&mutex_);
  auto it = entries_.constFind(token);
  if (it == entries_.constEnd()) return false;

  for (const StreamItem& candidate : it->items) {
    if (candidate.id == id) {
      *item = candidate;
      *peer = it->peer;
      return true;
    }
  }
  return false;
}
