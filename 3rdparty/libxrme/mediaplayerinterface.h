/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LIBXRME_MEDIAPLAYERINTERFACE_H
#define LIBXRME_MEDIAPLAYERINTERFACE_H

#include <xrme/common.h>

#include <QImage>
#include <QScopedPointer>

namespace xrme {

class MediaPlayerHandler;

class MediaPlayerInterface {
public:
  MediaPlayerInterface();
  virtual ~MediaPlayerInterface();

  // Control playback
  virtual void PlayPause() = 0;
  virtual void Stop() = 0;
  virtual void Next() = 0;
  virtual void Previous() = 0;

  // Query the current state of the player.  StateChanged() should be called
  // when any part of the state changes.
  virtual State state() const = 0;

  // Query the album art of the currently playing song.  This is separate from
  // state() because it is bigger and needs to be sent less often.
  // AlbumArtChanged() should be called when this changes.  Return a null
  // QImage() if there is no song playing or the song has no album art.
  virtual QImage album_art() const = 0;

  // Call when the values returned from the above getters have changed.
  virtual void StateChanged();
  virtual void AlbumArtChanged();

private:
  Q_DISABLE_COPY(MediaPlayerInterface);
  friend class MediaPlayerHandler;

  void Attach(MediaPlayerHandler* handler);

  struct Private;
  QScopedPointer<Private> d;
};

} // namespace xrme

#endif // LIBXRME_MEDIAPLAYERINTERFACE_H
