/* This file is part of Clementine.
   Copyright 2012, John Maguire <john.maguire@gmail.com>
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef CORE_QTFSLISTENER_H_
#define CORE_QTFSLISTENER_H_

#include <QFileSystemWatcher>

#include "filesystemwatcherinterface.h"

class QtFSListener : public FileSystemWatcherInterface {
  Q_OBJECT
 public:
  explicit QtFSListener(QObject* parent);
  virtual bool AddPath(const QString& path);
  virtual void RemovePath(const QString& path);
  virtual void Clear();

 private:
  QFileSystemWatcher watcher_;
};

#endif  // CORE_QTFSLISTENER_H_
