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

#include "audiosource.h"

#include <QFile>
#include <QtEndian>
#include <cmath>
#include <cstring>

#include "bytes.h"

namespace airplay {

int ToneSource::Read(char* data, int size) {
  int filled = 0;
  while (filled + kFrameSize <= size && frame_ < limit_) {
    const double t = double(frame_) / kSampleRate;
    const double freq = qint64(t) % 2 == 0 ? 440.0 : 660.0;
    const qint16 s = qint16(std::sin(t * freq * 2 * M_PI) * 8000.0);
    const QByteArray sample = Bytes(qToLittleEndian(s));
    std::memcpy(data + filled, sample.constData(), sample.size());
    std::memcpy(data + filled + 2, sample.constData(), sample.size());
    filled += kFrameSize;
    ++frame_;
  }
  return filled;
}

DeviceSource::DeviceSource(std::unique_ptr<QIODevice> device, qint64 frames)
    : device_(std::move(device)),
      remaining_bytes_(frames < 0 ? -1 : frames * kFrameSize) {}

int DeviceSource::Read(char* data, int size) {
  if (remaining_bytes_ >= 0) size = int(qMin<qint64>(size, remaining_bytes_));
  int filled = 0;
  // A QFile on a pipe blocks until there's data, and returns 0 at the end.
  while (filled < size) {
    const qint64 n = device_->read(data + filled, size - filled);
    if (n <= 0) break;
    filled += int(n);
  }
  if (remaining_bytes_ >= 0) remaining_bytes_ -= filled;
  return filled;
}

std::unique_ptr<QIODevice> OpenWav(const QString& path, QString* error) {
  std::unique_ptr<QFile> file(new QFile(path));
  if (!file->open(QIODevice::ReadOnly)) {
    *error = QString("%1: %2").arg(path, file->errorString());
    return nullptr;
  }
  const QByteArray riff = file->read(12);
  if (riff.size() < 12 || !riff.startsWith("RIFF") ||
      riff.mid(8, 4) != "WAVE") {
    *error = path + " isn't a WAV file";
    return nullptr;
  }
  while (true) {
    const QByteArray header = file->read(8);
    if (header.size() < 8) {
      *error = path + " has no data chunk";
      return nullptr;
    }
    const quint32 length = qFromLittleEndian<quint32>(header.constData() + 4);
    if (header.startsWith("data")) return file;
    const QByteArray body = file->read(length + (length & 1));
    if (header.startsWith("fmt ") && body.size() >= 16) {
      const int channels = qFromLittleEndian<quint16>(body.constData() + 2);
      const int rate = qFromLittleEndian<quint32>(body.constData() + 4);
      const int bits = qFromLittleEndian<quint16>(body.constData() + 14);
      if (channels != 2 || rate != kSampleRate || bits != 16) {
        *error = QString(
                     "%1 is %2 ch, %3 Hz, %4 bit; it needs to be 2 ch, "
                     "44100 Hz, 16 bit")
                     .arg(path)
                     .arg(channels)
                     .arg(rate)
                     .arg(bits);
        return nullptr;
      }
    }
  }
}

}  // namespace airplay
