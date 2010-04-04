/* This file is part of Clementine.

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

#ifndef VLCENGINE_H
#define VLCENGINE_H

#include "enginebase.h"

#include <vlc/vlc.h>

class QTimer;

class VlcEngine : public Engine::Base {
  Q_OBJECT

 public:
  VlcEngine();
  ~VlcEngine();

  bool init();

  bool canDecode( const QUrl &url ) const;

  bool load( const QUrl &url, bool stream = false );
  bool play( uint offset = 0 );
  void stop();
  void pause();
  void unpause();

  Engine::State state() const { return state_; }
  uint position() const;
  uint length() const;

  void seek( uint ms );

 protected:
  void setVolumeSW( uint percent );

 private:
  void HandleErrors() const;
  void AttachCallback(libvlc_event_manager_t* em, libvlc_event_type_t type,
                      libvlc_callback_t callback);
  static void StateChangedCallback(const libvlc_event_t* e, void* data);

 private:
  libvlc_exception_t exception_;
  libvlc_instance_t* instance_;
  libvlc_media_player_t* player_;

  Engine::State state_;
};

#endif // VLCENGINE_H
