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

#ifndef AIRPLAY_AUDIOSOURCE_H_
#define AIRPLAY_AUDIOSOURCE_H_

#include <QIODevice>
#include <QString>
#include <memory>

namespace airplay {

const int kSampleRate = 44100;
// Interleaved S16LE stereo.
const int kFrameSize = 4;

// Where the audio comes from: 44.1 kHz interleaved S16LE stereo.
class AudioSource {
 public:
  virtual ~AudioSource() {}
  // Fills up to |size| bytes of |data| and returns how many it filled, or 0
  // at the end.
  virtual int Read(char* data, int size) = 0;
};

// A tone that alternates between 440 and 660 Hz every second, so gaps are
// audible, for |frames| frames.
class ToneSource : public AudioSource {
 public:
  explicit ToneSource(qint64 frames) : limit_(frames) {}
  int Read(char* data, int size) override;

 private:
  qint64 frame_ = 0;
  qint64 limit_;
};

// Reads from |device|, which may block, for at most |frames| frames (or all
// of it if negative).
class DeviceSource : public AudioSource {
 public:
  DeviceSource(std::unique_ptr<QIODevice> device, qint64 frames);
  int Read(char* data, int size) override;

 private:
  std::unique_ptr<QIODevice> device_;
  qint64 remaining_bytes_;
};

// Opens a 16-bit 44.1 kHz stereo WAV file, positioned at its samples, or
// returns null and sets |error|.
std::unique_ptr<QIODevice> OpenWav(const QString& path, QString* error);

}  // namespace airplay

#endif  // AIRPLAY_AUDIOSOURCE_H_
