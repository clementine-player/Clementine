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

// Copyright: (C) 2003 Mark Kretschmann
//           (C) 2004,2005 Max Howell, <max.howell@methylblue.com>
// License:   See COPYING

#include "enginebase.h"

#include <QSettings>
#include <cmath>

#include "core/timeconstants.h"

const char* Engine::Base::kSettingsGroup = "Player";

Engine::Base::Base()
    : volume_(50),
      beginning_nanosec_(0),
      end_nanosec_(0),
      scope_(kScopeSize),
      fadeout_enabled_(true),
      fadeout_duration_nanosec_(2 * kNsecPerSec),  // 2s
      crossfade_enabled_(true),
      autocrossfade_enabled_(false),
      crossfade_same_album_(false),
      about_to_end_emitted_(false) {}

Engine::Base::~Base() {}

bool Engine::Base::Load(const MediaPlaybackRequest& req, TrackChangeFlags,
                        bool force_stop_at_end, quint64 beginning_nanosec,
                        qint64 end_nanosec) {
  Q_UNUSED(force_stop_at_end);

  playback_req_ = req;
  beginning_nanosec_ = beginning_nanosec;
  end_nanosec_ = end_nanosec;

  about_to_end_emitted_ = false;
  return true;
}

void Engine::Base::SetVolume(uint value) {
  volume_ = value;

  SetVolumeSW(MakeVolumeLogarithmic(value));
}

uint Engine::Base::MakeVolumeLogarithmic(uint volume) {
  // We're using a logarithmic function to make the volume ramp more natural.
  return static_cast<uint>(100 -
                           100.0 * std::log10((100 - volume) * 0.09 + 1.0));
}

void Engine::Base::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  fadeout_enabled_ = s.value("FadeoutEnabled", true).toBool();
  fadeout_duration_nanosec_ =
      s.value("FadeoutDuration", 2000).toLongLong() * kNsecPerMsec;
  crossfade_enabled_ = s.value("CrossfadeEnabled", true).toBool();
  autocrossfade_enabled_ = s.value("AutoCrossfadeEnabled", false).toBool();
  crossfade_same_album_ = !s.value("NoCrossfadeSameAlbum", true).toBool();
  fadeout_pause_enabled_ = s.value("FadeoutPauseEnabled", false).toBool();
  fadeout_pause_duration_nanosec_ =
      s.value("FadeoutPauseDuration", 250).toLongLong() * kNsecPerMsec;
}

void Engine::Base::EmitAboutToEnd() {
  if (about_to_end_emitted_) return;

  about_to_end_emitted_ = true;
  emit TrackAboutToEnd();
}

int Engine::Base::AddBackgroundStream(const QUrl& url) { return -1; }

bool Engine::Base::Play(const MediaPlaybackRequest& req, TrackChangeFlags c,
                        bool force_stop_at_end, quint64 beginning_nanosec,
                        qint64 end_nanosec) {
  if (!Load(req, c, force_stop_at_end, beginning_nanosec, end_nanosec))
    return false;

  return Play(0);
}
