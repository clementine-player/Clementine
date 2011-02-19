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

#ifndef XRME_COMMON_H
#define XRME_COMMON_H

#include <QString>

namespace xrme {

extern const char* kXmlnsXrme;
extern const char* kXmlnsXrmeMediaPlayer;
extern const char* kXmlnsXrmeRemoteControl;

struct Metadata {
  Metadata();

  QString title;
  QString artist;
  QString album;
  QString albumartist;
  QString composer;
  QString genre;
  int track;
  int disc;
  int year;
  int length_millisec;
  double rating; // range 0.0 - 1.0
};

struct State {
  State();

  enum PlaybackState {
    PlaybackState_Stopped = 0,
    PlaybackState_Playing = 1,
    PlaybackState_Paused = 2,
  };

  PlaybackState playback_state;
  int position_millisec;
  double volume; // range 0.0 - 1.0
  bool can_go_next;
  bool can_go_previous;
  bool can_seek;
  Metadata metadata;
};

} // namespace xrme

#endif // XRME_COMMON_H
