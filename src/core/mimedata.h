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

#ifndef CORE_MIMEDATA_H_
#define CORE_MIMEDATA_H_

#include <QMimeData>

class MimeData : public QMimeData {
  Q_OBJECT

 public:
  MimeData(bool clear = false, bool play_now = false, bool enqueue = false,
           bool open_in_new_playlist = false)
      : override_user_settings_(false),
        clear_first_(clear),
        play_now_(play_now),
        enqueue_now_(enqueue),
        open_in_new_playlist_(open_in_new_playlist),
        name_for_new_playlist_(QString()),
        from_doubleclick_(false) {}

  // If this is set then MainWindow will not touch any of the other flags.
  bool override_user_settings_;

  // If this is set then the playlist will be cleared before these songs
  // are inserted.
  bool clear_first_;

  // If this is set then the first item that is inserted will start playing
  // immediately.  Note: this is always overridden with the user's preference
  // if the MimeData goes via MainWindow, unless you set
  // override_user_settings_.
  bool play_now_;

  // If this is set then the items are added to the queue after being inserted.
  bool enqueue_now_;

  // If this is set then the items are inserted into a newly created playlist.
  bool open_in_new_playlist_;

  // This serves as a name for the new playlist in 'open_in_new_playlist_' mode.
  QString name_for_new_playlist_;

  // This can be set if this MimeData goes via MainWindow (ie. it is created
  // manually in a double-click).  The MainWindow will set the above flags to
  // the defaults set by the user.
  bool from_doubleclick_;

  // Returns a pretty name for a playlist containing songs described by this
  // MimeData
  // object. By pretty name we mean the value of 'name_for_new_playlist_' or
  // generic
  // "Playlist" string if the 'name_for_new_playlist_' attribute is empty.
  QString get_name_for_new_playlist() {
    return name_for_new_playlist_.isEmpty() ? tr("Playlist")
                                            : name_for_new_playlist_;
  }
};

#endif  // CORE_MIMEDATA_H_
