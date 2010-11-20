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

#ifndef PHONONENGINE_H
#define PHONONENGINE_H

#include "enginebase.h"

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>

class QTimer;

class PhononEngine : public Engine::Base {
  Q_OBJECT

 public:
  PhononEngine();
  ~PhononEngine();

  bool Init();

  bool CanDecode( const QUrl &url );

  bool Load( const QUrl &url, Engine::TrackChangeType change );
  bool Play( uint offset = 0 );
  void Stop();
  void Pause();
  void Unpause();

  Engine::State state() const;
  uint position() const;
  uint length() const;

  void Seek( uint ms );

 protected:
  void SetVolumeSW( uint percent );

 private slots:
  void PhononFinished();
  void PhononStateChanged(Phonon::State new_state);
  void StateTimeoutExpired();

 private:
  Phonon::MediaObject* media_object_;
  Phonon::AudioOutput* audio_output_;

  QTimer* state_timer_;

  qint64 seek_offset_;
};

#endif // PHONONENGINE_H
