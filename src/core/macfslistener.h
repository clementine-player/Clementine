/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
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

#ifndef CORE_MACFSLISTENER_H_
#define CORE_MACFSLISTENER_H_

#include <CoreServices/CoreServices.h>

#include <QObject>
#include <QSet>
#include <QTimer>

#include "filesystemwatcherinterface.h"

class MacFSListener : public FileSystemWatcherInterface {
  Q_OBJECT

 public:
  explicit MacFSListener(QObject* parent = nullptr);
  void Init();
  void AddPath(const QString& path);
  void RemovePath(const QString& path);
  void Clear();

 signals:
  void PathChanged(const QString& path);

 private slots:
  void UpdateStream();

 private:
  void UpdateStreamAsync();

  static void EventStreamCallback(ConstFSEventStreamRef stream, void* user_data,
                                  size_t num_events, void* event_paths,
                                  const FSEventStreamEventFlags event_flags[],
                                  const FSEventStreamEventId event_ids[]);

  CFRunLoopRef run_loop_;
  FSEventStreamRef stream_;

  QSet<QString> paths_;
  QTimer update_timer_;
};

#endif  // CORE_MACFSLISTENER_H_
