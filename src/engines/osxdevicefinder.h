/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef OSXDEVICEFINDER_H
#define OSXDEVICEFINDER_H

#include <QString>

#include "engines/devicefinder.h"

// Devices are identified by their CoreAudio UID, which stays the same across
// reconnects and restarts, unlike their AudioDeviceID. osxaudiosink takes it as
// its "unique-id".
class OsxDeviceFinder : public DeviceFinder {
 public:
  OsxDeviceFinder();

  virtual bool Initialise() { return true; }
  virtual QList<Device> ListDevices();

  // Returns the UID of the device with this AudioDeviceID, as older versions
  // saved it, or an empty string if there is no such device now.
  static QString UidForDeviceId(quint32 device_id);

  // Returns whether an output device with this UID is connected.
  static bool HasOutputDevice(const QString& uid);
};

#endif  // OSXDEVICEFINDER_H
