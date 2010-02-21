#ifndef PHONONENGINE_H
#define PHONONENGINE_H

#include "enginebase.h"

#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>

class PhononEngine : public Engine::Base {
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

 private:
  Phonon::MediaObject* media_object_;
  Phonon::AudioOutput* audio_output_;
};

#endif // PHONONENGINE_H
