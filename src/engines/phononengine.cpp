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

#include "phononengine.h"

#include <QTimer>

PhononEngine::PhononEngine()
  : media_object_(new Phonon::MediaObject(this)),
    audio_output_(new Phonon::AudioOutput(Phonon::MusicCategory, this)),
    state_timer_(new QTimer(this)),
    seek_offset_(-1)
{
  Phonon::createPath(media_object_, audio_output_);

  connect(media_object_, SIGNAL(finished()), SLOT(PhononFinished()));
  connect(media_object_, SIGNAL(stateChanged(Phonon::State,Phonon::State)), SLOT(PhononStateChanged(Phonon::State)));

  state_timer_->setSingleShot(true);
  connect(state_timer_, SIGNAL(timeout()), SLOT(StateTimeoutExpired()));
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
  // The seek happens in PhononStateChanged - phonon doesn't seem to change
  // currentTime() if we seek before we start playing :S
  seek_offset_ = offset;

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

void PhononEngine::setVolumeSW(uint) {
  audio_output_->setVolume(qreal(m_volume) / 100.0);
}

void PhononEngine::PhononFinished() {
  emit trackEnded();
}

void PhononEngine::PhononStateChanged(Phonon::State new_state) {
  if (new_state == Phonon::ErrorState) {
    emit error(media_object_->errorString());
  }
  if (new_state == Phonon::PlayingState && seek_offset_ != -1) {
    media_object_->seek(seek_offset_);
    seek_offset_ = -1;
  }

  // Don't emit the state change straight away
  state_timer_->start(100);
}

void PhononEngine::StateTimeoutExpired() {
  emit stateChanged(state());
}
