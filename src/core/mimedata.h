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

#ifndef MIMEDATA_H
#define MIMEDATA_H

#include <QMimeData>

class MimeData : public QMimeData {
  Q_OBJECT

public:
  MimeData(bool clear = false, bool play_now = false, bool enqueue = false)
    : clear_first_(clear),
      play_now_(play_now),
      enqueue_now_(enqueue),
      from_doubleclick_(false) {}

  // If this is set then the playlist will be cleared before these songs
  // are inserted.
  bool clear_first_;

  // If this is set then the first item that is inserted will start playing
  // immediately.  Note: this is always overridden with the user's preference
  // if the MimeData goes via MainWindow.
  bool play_now_;

  // If this is set then the items are added to the queue after being inserted.
  bool enqueue_now_;

  // This can be set if this MimeData goes via MainWindow (ie. it is created
  // manually in a double-click).  The MainWindow will set the above three
  // flags to the defaults set by the user.
  bool from_doubleclick_;
};

#endif // MIMEDATA_H
