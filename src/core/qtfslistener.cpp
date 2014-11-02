/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>

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

#include "qtfslistener.h"

#include <QStringList>

QtFSListener::QtFSListener(QObject* parent)
    : FileSystemWatcherInterface(parent), watcher_(this) {
  connect(&watcher_, SIGNAL(directoryChanged(const QString&)),
          SIGNAL(PathChanged(const QString&)));
}

void QtFSListener::AddPath(const QString& path) { watcher_.addPath(path); }

void QtFSListener::RemovePath(const QString& path) {
  watcher_.removePath(path);
}

void QtFSListener::Clear() {
  watcher_.removePaths(watcher_.directories());
  watcher_.removePaths(watcher_.files());
}
