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

#include "vlcengine.h"
#include "vlcscopedref.h"

#include <QTimer>
#include <QtDebug>

#include <boost/bind.hpp>

VlcEngine::VlcEngine()
  : instance_(NULL),
    player_(NULL),
    state_(Engine::Empty)
{
  static const char * const args[] = {
    "-I", "dummy",        // Don't use any interface
    "--ignore-config",    // Don't use VLC's config
    "--extraintf=logger", // log anything
    "--verbose=2",        // be much more verbose then normal for debugging purpose
    //"--plugin-path=C:\\vlc-0.9.9-win32\\plugins\\"

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    "--aout=alsa",        // The default, pulseaudio, is buggy
#endif
  };

  // Create the VLC instance
  libvlc_exception_init(&exception_);
  instance_ = libvlc_new(sizeof(args) / sizeof(args[0]), args, &exception_);
  HandleErrors();

  // Create the media player
  player_ = libvlc_media_player_new(instance_, &exception_);
  HandleErrors();

  // Add event handlers
  libvlc_event_manager_t* player_em =
      libvlc_media_player_event_manager(player_, &exception_);
  HandleErrors();

  AttachCallback(player_em, libvlc_MediaPlayerEncounteredError, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerNothingSpecial, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerOpening, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerBuffering, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerPlaying, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerPaused, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerStopped, StateChangedCallback);
  AttachCallback(player_em, libvlc_MediaPlayerEndReached, StateChangedCallback);
  HandleErrors();
}

VlcEngine::~VlcEngine() {
  libvlc_media_player_stop(player_, &exception_);
  libvlc_media_player_release(player_);
  libvlc_release(instance_);
  HandleErrors();
}

void VlcEngine::AttachCallback(libvlc_event_manager_t* em, libvlc_event_type_t type,
                               libvlc_callback_t callback) {
  libvlc_event_attach(em, type, callback, this, &exception_);
  HandleErrors();
}

void VlcEngine::StateChangedCallback(const libvlc_event_t* e, void* data) {
  VlcEngine* engine = reinterpret_cast<VlcEngine*>(data);

  switch (e->type) {
    case libvlc_MediaPlayerNothingSpecial:
    case libvlc_MediaPlayerStopped:
    case libvlc_MediaPlayerEncounteredError:
      engine->state_ = Engine::Empty;
      break;

    case libvlc_MediaPlayerOpening:
    case libvlc_MediaPlayerBuffering:
    case libvlc_MediaPlayerPlaying:
      engine->state_ = Engine::Playing;
      break;

    case libvlc_MediaPlayerPaused:
      engine->state_ = Engine::Paused;
      break;

    case libvlc_MediaPlayerEndReached:
      engine->state_ = Engine::Idle;
      break;
  }

  emit engine->stateChanged(engine->state_);
}

bool VlcEngine::init() {
  return true;
}

bool VlcEngine::canDecode(const QUrl &url) const {
  // TODO
  return true;
}

bool VlcEngine::load(const QUrl &url, bool stream) {
  VlcScopedRef<libvlc_media_t> media(
      libvlc_media_new(instance_, url.toEncoded().constData(), &exception_));

  if (libvlc_exception_raised(&exception_))
    return false;

  libvlc_media_player_set_media(player_, media, &exception_);
  if (libvlc_exception_raised(&exception_))
    return false;

  return true;
}

bool VlcEngine::play(uint offset) {
  libvlc_media_player_play(player_, &exception_);
  if (libvlc_exception_raised(&exception_))
    return false;

  seek(offset);

  return true;
}

void VlcEngine::stop() {
  libvlc_media_player_stop(player_, &exception_);
  HandleErrors();
}

void VlcEngine::pause() {
  libvlc_media_player_pause(player_, &exception_);
  HandleErrors();
}

void VlcEngine::unpause() {
  libvlc_media_player_play(player_, &exception_);
  HandleErrors();
}

uint VlcEngine::position() const {
  bool is_playing = libvlc_media_player_is_playing(
      player_, const_cast<libvlc_exception_t*>(&exception_));
  HandleErrors();

  if (!is_playing)
    return 0;

  float pos = libvlc_media_player_get_position(
      player_, const_cast<libvlc_exception_t*>(&exception_));
  HandleErrors();

  return pos * length();
}

uint VlcEngine::length() const {
  bool is_playing = libvlc_media_player_is_playing(
      player_, const_cast<libvlc_exception_t*>(&exception_));
  HandleErrors();

  if (!is_playing)
    return 0;

  libvlc_time_t len = libvlc_media_player_get_length(
      player_, const_cast<libvlc_exception_t*>(&exception_));
  HandleErrors();

  return len;
}

void VlcEngine::seek(uint ms) {
  uint len = length();
  if (len == 0)
    return;

  float pos = float(ms) / len;

  libvlc_media_player_set_position(player_, pos, &exception_);
  HandleErrors();
}

void VlcEngine::setVolumeSW(uint volume) {
  libvlc_audio_set_volume(instance_, volume, &exception_);
  HandleErrors();
}

void VlcEngine::HandleErrors() const {
  if (libvlc_exception_raised(&exception_)) {
    qFatal("libvlc error: %s", libvlc_exception_get_message(&exception_));
  }
}
