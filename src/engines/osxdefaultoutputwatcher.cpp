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

#include "osxdefaultoutputwatcher.h"

#include <QMetaObject>
#include <QTimer>

#include "core/logging.h"

namespace {

// macOS can report several changes while it switches, especially to or from a
// Bluetooth device, so wait for them to stop before reopening the sink.
const int kSettleMsec = 500;

const AudioObjectPropertyAddress kDefaultOutputAddress = {
    kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster};

const AudioObjectPropertyAddress kDevicesAddress = {
    kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster};

}  // namespace

OsxDefaultOutputWatcher::OsxDefaultOutputWatcher(QObject* parent)
    : QObject(parent),
      settle_timer_(new QTimer(this)),
      devices_settle_timer_(new QTimer(this)),
      device_id_(CurrentDefaultOutput()),
      listening_(false),
      listening_for_devices_(false) {
  settle_timer_->setSingleShot(true);
  settle_timer_->setInterval(kSettleMsec);
  connect(settle_timer_, SIGNAL(timeout()), SLOT(CheckDefaultOutput()));

  devices_settle_timer_->setSingleShot(true);
  devices_settle_timer_->setInterval(kSettleMsec);
  connect(devices_settle_timer_, SIGNAL(timeout()), SIGNAL(DevicesChanged()));

  const OSStatus status = AudioObjectAddPropertyListener(
      kAudioObjectSystemObject, &kDefaultOutputAddress, PropertyListener, this);
  if (status == kAudioHardwareNoError) {
    listening_ = true;
  } else {
    qLog(Warning) << "Can't watch the default audio output:" << status;
  }

  const OSStatus devices_status = AudioObjectAddPropertyListener(
      kAudioObjectSystemObject, &kDevicesAddress, PropertyListener, this);
  if (devices_status == kAudioHardwareNoError) {
    listening_for_devices_ = true;
  } else {
    qLog(Warning) << "Can't watch the audio devices:" << devices_status;
  }
}

OsxDefaultOutputWatcher::~OsxDefaultOutputWatcher() {
  if (listening_) {
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject,
                                      &kDefaultOutputAddress, PropertyListener,
                                      this);
  }
  if (listening_for_devices_) {
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject,
                                      &kDevicesAddress, PropertyListener, this);
  }
}

OSStatus OsxDefaultOutputWatcher::PropertyListener(
    AudioObjectID, UInt32 address_count,
    const AudioObjectPropertyAddress* addresses, void* client_data) {
  // This runs on a CoreAudio thread.
  OsxDefaultOutputWatcher* me =
      reinterpret_cast<OsxDefaultOutputWatcher*>(client_data);
  for (UInt32 i = 0; i < address_count; ++i) {
    if (addresses[i].mSelector == kAudioHardwarePropertyDevices) {
      QMetaObject::invokeMethod(me, "DevicesNotified", Qt::QueuedConnection);
    } else {
      QMetaObject::invokeMethod(me, "Notified", Qt::QueuedConnection);
    }
  }
  return kAudioHardwareNoError;
}

void OsxDefaultOutputWatcher::Notified() { settle_timer_->start(); }

void OsxDefaultOutputWatcher::DevicesNotified() {
  devices_settle_timer_->start();
}

void OsxDefaultOutputWatcher::CheckDefaultOutput() {
  const AudioDeviceID device_id = CurrentDefaultOutput();
  if (device_id == kAudioDeviceUnknown || device_id == device_id_) return;

  qLog(Info) << "Default audio output changed from" << device_id_ << "to"
             << device_id;
  device_id_ = device_id;
  emit DefaultOutputChanged();
}

AudioDeviceID OsxDefaultOutputWatcher::CurrentDefaultOutput() {
  AudioDeviceID device_id = kAudioDeviceUnknown;
  UInt32 size_bytes = sizeof(device_id);
  const OSStatus status = AudioObjectGetPropertyData(
      kAudioObjectSystemObject, &kDefaultOutputAddress, 0, nullptr, &size_bytes,
      &device_id);
  if (status != kAudioHardwareNoError) {
    qLog(Warning) << "Can't get the default audio output:" << status;
    return kAudioDeviceUnknown;
  }
  return device_id;
}
