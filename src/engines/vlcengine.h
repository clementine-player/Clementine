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

#ifndef VLCENGINE_H
#define VLCENGINE_H

#include "enginebase.h"

#include <vlc/vlc.h>
#include <boost/circular_buffer.hpp>

#include <QMutex>

class QTimer;

class VlcEngine : public Engine::Base {
  Q_OBJECT

 public:
  VlcEngine();
  ~VlcEngine();

  bool Init();

  bool CanDecode( const QUrl &url );

  bool Load( const QUrl &url, Engine::TrackChangeType change );
  bool Play( uint offset = 0 );
  void Stop();
  void Pause();
  void Unpause();

  Engine::State state() const { return state_; }
  uint position() const;
  uint length() const;

  void Seek( uint ms );

  static void SetScopeData(float* data, int size);
  const Engine::Scope& Scope();

 protected:
  void SetVolumeSW( uint percent ); 

 private:
  void HandleErrors() const;
  void AttachCallback(libvlc_event_manager_t* em, libvlc_event_type_t type,
                      libvlc_callback_t callback);
  static void StateChangedCallback(const libvlc_event_t* e, void* data);

 private:
  // The callbacks need access to this
  static VlcEngine* sInstance;

  // VLC bits and pieces
  libvlc_exception_t exception_;
  libvlc_instance_t* instance_;
  libvlc_media_player_t* player_;

  // Our clementine_scope VLC plugin puts data in here
  QMutex scope_mutex_;
  boost::circular_buffer<float> scope_data_;

  Engine::State state_;
};

#endif // VLCENGINE_H
