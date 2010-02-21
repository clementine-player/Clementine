#include "phononengine.h"

PhononEngine::PhononEngine()
  : media_object_(new Phonon::MediaObject(this)),
    audio_output_(new Phonon::AudioOutput(Phonon::MusicCategory, this))
{
  Phonon::createPath(media_object_, audio_output_);
}

PhononEngine::~PhononEngine() {
  delete media_object_;
  delete audio_output_;
}

bool PhononEngine::init() {
  return true;
}

bool PhononEngine::canDecode(const QUrl &url) const {
  // TODO
  return true;
}

bool PhononEngine::load(const QUrl &url, bool stream) {
  media_object_->setCurrentSource(Phonon::MediaSource(url));
  return true;
}

bool PhononEngine::play(uint offset) {
  media_object_->play();
  return true;
}

void PhononEngine::stop() {
  media_object_->stop();
}

void PhononEngine::pause() {
  media_object_->pause();
}

void PhononEngine::unpause() {
  media_object_->play();
}

Engine::State PhononEngine::state() const {
  switch (media_object_->state()) {
    case Phonon::LoadingState:
      return Engine::Idle;

    case Phonon::PlayingState:
    case Phonon::BufferingState:
      return Engine::Playing;

    case Phonon::PausedState:
      return Engine::Paused;

    case Phonon::StoppedState:
    case Phonon::ErrorState:
    default:
      return Engine::Empty;
  }
}

uint PhononEngine::position() const {
  return media_object_->currentTime();
}

uint PhononEngine::length() const {
  return media_object_->totalTime();
}

void PhononEngine::seek(uint ms) {
  media_object_->seek(ms);
}

void PhononEngine::setVolumeSW(uint percent) {
  audio_output_->setVolume(qreal(percent) / 100.0);
}
