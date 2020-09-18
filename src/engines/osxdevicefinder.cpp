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

#include "osxdevicefinder.h"

#include <CoreAudio/AudioHardware.h>

#include <memory>

#include "core/logging.h"
#include "core/scoped_cftyperef.h"

namespace {

template <typename T>
std::unique_ptr<T> GetProperty(const AudioDeviceID& device_id,
                               const AudioObjectPropertyAddress& address,
                               UInt32* size_bytes_out = nullptr) {
  UInt32 size_bytes = 0;
  OSStatus status =
      AudioObjectGetPropertyDataSize(device_id, &address, 0, NULL, &size_bytes);
  if (status != kAudioHardwareNoError) {
    qLog(Warning) << "AudioObjectGetPropertyDataSize failed:" << status;
    return std::unique_ptr<T>();
  }

  std::unique_ptr<T> ret(reinterpret_cast<T*>(malloc(size_bytes)));

  status = AudioObjectGetPropertyData(device_id, &address, 0, NULL, &size_bytes,
                                      ret.get());
  if (status != kAudioHardwareNoError) {
    qLog(Warning) << "AudioObjectGetPropertyData failed:" << status;
    return std::unique_ptr<T>();
  }

  if (size_bytes_out) {
    *size_bytes_out = size_bytes;
  }

  return ret;
}

}  // namespace

OsxDeviceFinder::OsxDeviceFinder() : DeviceFinder("osxaudiosink") {}

QList<DeviceFinder::Device> OsxDeviceFinder::ListDevices() {
  QList<Device> ret;

  AudioObjectPropertyAddress address = {kAudioHardwarePropertyDevices,
                                        kAudioObjectPropertyScopeGlobal,
                                        kAudioObjectPropertyElementMaster};

  UInt32 device_size_bytes = 0;
  std::unique_ptr<AudioDeviceID> devices = GetProperty<AudioDeviceID>(
      kAudioObjectSystemObject, address, &device_size_bytes);
  if (!devices.get()) {
    return ret;
  }
  const int device_count = device_size_bytes / sizeof(AudioDeviceID);

  address.mScope = kAudioDevicePropertyScopeOutput;
  for (UInt32 i = 0; i < device_count; ++i) {
    const AudioDeviceID id = devices.get()[i];

    // Query device name
    address.mSelector = kAudioDevicePropertyDeviceNameCFString;
    std::unique_ptr<CFStringRef> device_name =
        GetProperty<CFStringRef>(id, address);
    ScopedCFTypeRef<CFStringRef> scoped_device_name(*device_name.get());
    if (!device_name.get()) {
      continue;
    }

    // Determine if the device is an output device (it is an output device if
    // it has output channels)
    address.mSelector = kAudioDevicePropertyStreamConfiguration;
    std::unique_ptr<AudioBufferList> buffer_list =
        GetProperty<AudioBufferList>(id, address);
    if (!buffer_list.get() || buffer_list->mNumberBuffers == 0) {
      continue;
    }

    Device dev;
    dev.description = QString::fromUtf8(
        CFStringGetCStringPtr(*device_name, CFStringGetSystemEncoding()));
    if (dev.description.isEmpty()) dev.description = QString("Unknown device");
    dev.device_property_value = id;
    dev.icon_name = GuessIconName(dev.description);
    ret.append(dev);
  }
  return ret;
}
