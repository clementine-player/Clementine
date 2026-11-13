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

#ifndef ENGINES_OSXDEFAULTOUTPUTWATCHER_H
#define ENGINES_OSXDEFAULTOUTPUTWATCHER_H

#include <CoreAudio/AudioHardware.h>

#include <QObject>

class QTimer;

// Tells when macOS's default audio output device changes, for example when
// the user picks other speakers in System Settings or connects headphones.
// osxaudiosink picks the default device only when it opens, so a sink that's
// already playing has to be reopened to follow the change.
class OsxDefaultOutputWatcher : public QObject {
  Q_OBJECT

 public:
  explicit OsxDefaultOutputWatcher(QObject* parent = nullptr);
  ~OsxDefaultOutputWatcher() override;

 signals:
  // Emitted once the default output has settled on a different device.
  void DefaultOutputChanged();

 private slots:
  void Notified();
  void CheckDefaultOutput();

 private:
  static OSStatus PropertyListener(AudioObjectID object_id,
                                   UInt32 address_count,
                                   const AudioObjectPropertyAddress* addresses,
                                   void* client_data);
  static AudioDeviceID CurrentDefaultOutput();

  QTimer* settle_timer_;
  AudioDeviceID device_id_;
  bool listening_;
};

#endif  // ENGINES_OSXDEFAULTOUTPUTWATCHER_H
