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
};

#endif // PHONONENGINE_H
