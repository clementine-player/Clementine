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

//Copyright: (C) 2003 Mark Kretschmann
//           (C) 2004,2005 Max Howell, <max.howell@methylblue.com>
//License:   See COPYING

#include "enginebase.h"

#include <cmath>

#include <QSettings>

const char* Engine::Base::kSettingsGroup = "Player";

Engine::Base::Base()
  : volume_(50),
    scope_(kScopeSize),
    fadeout_enabled_(true),
    fadeout_duration_(2000),
    crossfade_enabled_(true),
    crossfade_next_track_(false)
{
}

Engine::Base::~Base() {
}

bool Engine::Base::Load(const QUrl &url) {
  url_ = url;
  return true;
}

void Engine::Base::SetVolume(uint value) {
  volume_ = value;

  SetVolumeSW(MakeVolumeLogarithmic(value));
}

uint Engine::Base::MakeVolumeLogarithmic(uint volume) {
  // We're using a logarithmic function to make the volume ramp more natural.
  return static_cast<uint>( 100 - 100.0 * std::log10( ( 100 - volume ) * 0.09 + 1.0 ) );
}

void Engine::Base::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  fadeout_enabled_ = s.value("FadeoutEnabled", true).toBool();
  fadeout_duration_ = s.value("FadeoutDuration", 2000).toInt();
  crossfade_enabled_ = s.value("CrossfadeEnabled", true).toBool();
}
