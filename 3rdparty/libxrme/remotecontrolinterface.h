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

#ifndef XRME_REMOTECONTROLINTERFACE_H
#define XRME_REMOTECONTROLINTERFACE_H

#include "common.h"
#include "connection.h"

class QImage;

namespace xrme {

class RemoteControlHandler;

class RemoteControlInterface {
public:
  RemoteControlInterface();
  virtual ~RemoteControlInterface();

  // All functions here will work asynchronously and return immediately.

  // Call these to control the playback of a MediaPlayer.
  void PlayPause(const QString& peer_jid_resource);
  void Stop(const QString& peer_jid_resource);
  void Next(const QString& peer_jid_resource);
  void Previous(const QString& peer_jid_resource);

  // Call this to query the MediaPlayer.  StateChanged() will be called later.
  void QueryState(const QString& peer_jid_resource);

  // Called whenever the MediaPlayer's state changes.
  virtual void StateChanged(const QString& peer_jid_resource,
                            const State& state) = 0;

  // Called whenever the MediaPlayer's album art changes.
  virtual void AlbumArtChanged(const QString& peer_jid_resource,
                               const QImage& art) = 0;

private:
  Q_DISABLE_COPY(RemoteControlInterface);
  friend class RemoteControlHandler;

  void Attach(RemoteControlHandler* handler);

  struct Private;
  QScopedPointer<Private> d;
};

} // namespace xrme

#endif // XRME_REMOTECONTROLINTERFACE_H
