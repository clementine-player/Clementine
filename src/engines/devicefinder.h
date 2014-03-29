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

#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include <QStringList>
#include <QVariant>

// Finds audio output devices that can be used with a given gstreamer sink.
class DeviceFinder {
 public:
  struct Device {
    // The value to set as the "device" gstreamer property.
    QVariant device_property_value;

    // A human readable description of the device.
    QString description;

    // An icon to use in the UI.
    QString icon_name;
  };

  virtual ~DeviceFinder() {}

  // The name of the gstreamer sink element that devices found by this class
  // can be used with.
  QString gstreamer_sink() const { return gstreamer_sink_; }

  // Does any necessary setup, returning false if this DeviceFinder cannot
  // be used.
  virtual bool Initialise() = 0;

  // Returns a list of available devices.
  virtual QList<Device> ListDevices() = 0;

 protected:
  explicit DeviceFinder(const QString& gstreamer_sink);

  QString GuessIconName(const QString& description) const;

 private:
  QString gstreamer_sink_;
};

#endif // DEVICEFINDER_H
