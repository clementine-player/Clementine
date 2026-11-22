/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef ENGINES_GSTDECODEAUDIOONLY_H_
#define ENGINES_GSTDECODEAUDIOONLY_H_

#include <gst/gst.h>

// Sets up a decodebin or uridecodebin to decode only a file's audio streams.
//
// By default they decode every stream they can, so a file with a video track
// (a music video, or an M4A with its cover as a video track) gets a video
// decoder too, often a hardware one, for pictures nothing shows. Its pads are
// still exposed, and Clementine's pad-added callbacks expect audio. With
// this, decodebin doesn't make decoders that can't output raw audio, and
// exposes only audio pads.
void GstDecodeAudioOnly(GstElement* decodebin);

#endif  // ENGINES_GSTDECODEAUDIOONLY_H_
