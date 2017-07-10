/* This file is part of Clementine.
   Copyright 2017, Adam Borowski <kilobyte@angband.pl>

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

#include <QFile>

#include "alsadevicefinder.h"

AlsaDeviceFinder::AlsaDeviceFinder()
    : DeviceFinder("alsasink") {
}

QList<DeviceFinder::Device> AlsaDeviceFinder::ListDevices() {
  QList<Device> ret;

  QFile cards("/proc/asound/cards");
  if (!cards.open(QIODevice::ReadOnly)) return ret;

/* Syntax:
                        snd_iprintf(buffer, "%2i [%-15s]: %s - %s\n",
                                        idx,
                                        card->id,
                                        card->driver,
                                        card->shortname);
                        snd_iprintf(buffer, "                      %s\n",
                                        card->longname);
*/
  QRegExp regid("^ ?(\\d{1,2}) \\[.{15}\\]: .* - .*$");
  QRegExp regln("^                      (.*)\n");

  while (1) {
    QString line = cards.readLine();
    if (regid.indexIn(line) == -1) break;
    line = cards.readLine();
    if (regln.indexIn(line) == -1) break;

    Device dev;
    dev.description = regln.cap(1).remove(QRegExp(" at .*$"));
    dev.device_property_value = QString("hw:%1").arg(regid.cap(1));
    dev.icon_name = GuessIconName(dev.description);
    ret.append(dev);
  }

  return ret;
}
