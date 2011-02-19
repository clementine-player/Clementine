/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

namespace xrme {

const char* kXmlnsXrme = "http://purplehatstands.com/xmlns/xrme";
const char* kXmlnsXrmeMediaPlayer = "http://purplehatstands.com/xmlns/xrme/mediaplayer";
const char* kXmlnsXrmeRemoteControl = "http://purplehatstands.com/xmlns/xrme/remotecontrol";

Metadata::Metadata()
    : track(0),
      disc(0),
      year(0),
      length_millisec(0),
      rating(0.0) {
}

State::State()
    : playback_state(PlaybackState_Stopped),
      position_millisec(0),
      volume(0.0),
      can_go_next(false),
      can_go_previous(false),
      can_seek(false) {
}

} // namespace xrme
