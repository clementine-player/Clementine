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

  bool init();

  bool canDecode( const QUrl &url ) const;

  bool load( const QUrl &url, bool stream = false );
  bool play( uint offset = 0 );
  void stop();
  void pause();
  void unpause();

  Engine::State state() const;
  uint position() const;
  uint length() const;

  void seek( uint ms );

 protected:
  void setVolumeSW( uint percent );

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
