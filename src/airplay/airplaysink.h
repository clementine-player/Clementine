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

#ifndef AIRPLAY_AIRPLAYSINK_H_
#define AIRPLAY_AIRPLAYSINK_H_

#include <QString>
#include <memory>

// Where an airplaysink sends what it renders. Called from GStreamer's
// streaming thread.
class AirPlaySinkTarget {
 public:
  virtual ~AirPlaySinkTarget() {}

  // 44.1 kHz S16LE stereo, delivered in real time.
  virtual void Write(const char* pcm, int size) = 0;
  // The pipeline was flushed, by a seek.
  virtual void Flushed() = 0;
};

// "clementineairplaysink", a GStreamer sink that hands what it renders to
// the AirPlaySinkTarget registered under its "device" property. Every
// pipeline of an AirPlayEngine ends in one, and they all feed the engine's
// one session.
namespace AirPlaySink {

extern const char kFactoryName[];

// Registers the element with GStreamer, once.
void RegisterElement();

// Makes |target| receive what sinks with |device| render, until it's
// replaced or unregistered.
void SetTarget(const QString& device,
               std::shared_ptr<AirPlaySinkTarget> target);
void RemoveTarget(const QString& device);

}  // namespace AirPlaySink

#endif  // AIRPLAY_AIRPLAYSINK_H_
