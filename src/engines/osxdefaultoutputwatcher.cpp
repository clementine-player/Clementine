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

}  // namespace

OsxDefaultOutputWatcher::OsxDefaultOutputWatcher(QObject* parent)
    : QObject(parent),
      settle_timer_(new QTimer(this)),
      device_id_(CurrentDefaultOutput()),
      listening_(false) {
  settle_timer_->setSingleShot(true);
  settle_timer_->setInterval(kSettleMsec);
  connect(settle_timer_, SIGNAL(timeout()), SLOT(CheckDefaultOutput()));

  const OSStatus status = AudioObjectAddPropertyListener(
      kAudioObjectSystemObject, &kDefaultOutputAddress, PropertyListener, this);
  if (status == kAudioHardwareNoError) {
    listening_ = true;
  } else {
    qLog(Warning) << "Can't watch the default audio output:" << status;
  }
}

OsxDefaultOutputWatcher::~OsxDefaultOutputWatcher() {
  if (listening_) {
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject,
                                      &kDefaultOutputAddress, PropertyListener,
                                      this);
  }
}

OSStatus OsxDefaultOutputWatcher::PropertyListener(
    AudioObjectID, UInt32, const AudioObjectPropertyAddress*,
    void* client_data) {
  // This runs on a CoreAudio thread.
  QMetaObject::invokeMethod(
      reinterpret_cast<OsxDefaultOutputWatcher*>(client_data), "Notified",
      Qt::QueuedConnection);
  return kAudioHardwareNoError;
}

void OsxDefaultOutputWatcher::Notified() { settle_timer_->start(); }

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
