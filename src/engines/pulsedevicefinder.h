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

#ifndef PULSEDEVICEFINDER_H
#define PULSEDEVICEFINDER_H

#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/mainloop.h>

#include <QMutex>

#include "engines/devicefinder.h"

class PulseDeviceFinder : public DeviceFinder {
 public:
  PulseDeviceFinder();
  ~PulseDeviceFinder();

  virtual bool Initialise();
  virtual QList<Device> ListDevices();

 private:
  struct ListDevicesState {
    ListDevicesState() : finished(false) {}

    bool finished;
    QList<Device> devices;
  };

  bool Reconnect();

  static void GetSinkInfoCallback(pa_context* c,
                                  const pa_sink_info* info,
                                  int eol,
                                  void* state_voidptr);

  pa_mainloop* mainloop_;
  pa_context* context_;
};

#endif // PULSEDEVICEFINDER_H
