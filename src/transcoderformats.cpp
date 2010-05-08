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

#include "transcoderformats.h"

QStringList OggVorbisTranscoder::gst_elements() const {
  return QStringList() << "vorbisenc" << "oggmux";
}

QStringList OggSpeexTranscoder::gst_elements() const {
  return QStringList() << "speexenc" << "oggmux";
}

QStringList FlacTranscoder::gst_elements() const {
  return QStringList() << "flacenc";
}

QStringList Mp3Transcoder::gst_elements() const {
  return QStringList() << "lamemp3enc";
}

QStringList M4aTranscoder::gst_elements() const {
  return QStringList() << "faac" << "ffmux_mp4";
}

QStringList ThreeGPTranscoder::gst_elements() const {
  return QStringList() << "faac" << "ffmux_3gp";
}
