/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "core/logging.h"
#include "engines/pulsedevicefinder.h"

PulseDeviceFinder::PulseDeviceFinder()
    : DeviceFinder("pulseaudiosink"),
      mainloop_(nullptr),
      context_(nullptr) {
}

bool PulseDeviceFinder::Initialise() {
  mainloop_ = pa_mainloop_new();
  if (!mainloop_) {
    qLog(Warning) << "Failed to create pulseaudio mainloop";
    return false;
  }

  return Reconnect();
}

bool PulseDeviceFinder::Reconnect() {
  if (context_) {
    pa_context_disconnect(context_);
    pa_context_unref(context_);
  }

  context_ = pa_context_new(pa_mainloop_get_api(mainloop_),
                            "Clementine device finder");
  if (!context_) {
    qLog(Warning) << "Failed to create pulseaudio context";
    return false;
  }

  if (pa_context_connect(context_, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
    qLog(Warning) << "Failed to connect pulseaudio context";
    return false;
  }

  // Wait for the context to be connected.
  forever {
    const pa_context_state state = pa_context_get_state(context_);
    if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
      qLog(Warning) << "Connection to pulseaudio failed";
      return false;
    }

    if (state == PA_CONTEXT_READY) {
      return true;
    }

    pa_mainloop_iterate(mainloop_, true, nullptr);
  }
}

QList<DeviceFinder::Device> PulseDeviceFinder::ListDevices() {
  if (!context_ || pa_context_get_state(context_) != PA_CONTEXT_READY) {
    return QList<Device>();
  }

retry:
  ListDevicesState state;
  pa_context_get_sink_info_list(
      context_, &PulseDeviceFinder::GetSinkInfoCallback, &state);

  forever {
    if (state.finished) {
      return state.devices;
    }

    switch (pa_context_get_state(context_)) {
    case PA_CONTEXT_READY:
      break;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
      // Maybe pulseaudio died.  Try reconnecting.
      if (Reconnect()) {
        goto retry;
      }
      return state.devices;
    default:
      return state.devices;
    }

    pa_mainloop_iterate(mainloop_, true, nullptr);
  }
}

void PulseDeviceFinder::GetSinkInfoCallback(pa_context* c,
                                            const pa_sink_info* info,
                                            int eol,
                                            void* state_voidptr) {
  ListDevicesState* state = reinterpret_cast<ListDevicesState*>(state_voidptr);

  if (info) {
    Device dev;
    dev.name = QString::fromUtf8(info->name);
    dev.description = QString::fromUtf8(info->description);
    dev.icon_name = QString::fromUtf8(
        pa_proplist_gets(info->proplist, "device.icon_name"));

    state->devices.append(dev);
  }

  if (eol) {
    state->finished = true;
  }
}

PulseDeviceFinder::~PulseDeviceFinder() {
  if (context_) {
    pa_context_disconnect(context_);
    pa_context_unref(context_);
  }

  if (mainloop_) {
    pa_mainloop_free(mainloop_);
  }
}
