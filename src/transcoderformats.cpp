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

GstElement* OggVorbisTranscoder::CreateEncodeBin() const {
  return CreateBin(QStringList() << "vorbisenc" << "oggmux");
}

GstElement* OggSpeexTranscoder::CreateEncodeBin() const {
  return CreateBin(QStringList() << "speexenc" << "oggmux");
}

GstElement* FlacTranscoder::CreateEncodeBin() const {
  return CreateElement("flacenc");
}

GstElement* Mp3Transcoder::CreateEncodeBin() const {
  return CreateElement("lamemp3enc");
}

GstElement* M4aTranscoder::CreateEncodeBin() const {
  return CreateBin(QStringList() << "faac" << "ffmux_mp4");
}

GstElement* ThreeGPTranscoder::CreateEncodeBin() const {
  return CreateBin(QStringList() << "faac" << "ffmux_3gp");
}
