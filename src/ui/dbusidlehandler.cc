/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2015, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

namespace {
  
  // Gnome screensaver 
  const char* kGnomeScreensaverService
      = "org.gnome.ScreenSaver";
  const char* kGnomeScreensaverPath
      = "/";
  const char* kGnomeScreensaverInterface
      = "org.gnome.ScreenSaver";

  // Freedesktop screensaver
  const char* kFreedesktopScreensaverService
      = "org.freedesktop.ScreenSaver";
  const char* kFreedesktopScreensaverPath
      = "/ScreenSaver";
  const char* kFreedesktopScreensaverInterface
      = "org.freedesktop.ScreenSaver";

  // Gnome powermanager
  const char* kGnomePowermanagerService
      = "org.gnome.SessionManager";
  const char* kGnomePowermanagerPath
      = "/org/gnome/SessionManager";
  const char* kGnomePowermanagerInterface
      = "org.gnome.SessionManager";

  // Freedesktop powermanager
  const char* kFreedesktopPowermanagerService
      = "org.freedesktop.PowerManagement";
  const char* kFreedesktopPowermanagerPath
      = "/org/freedesktop/PowerManagement/Inhibit";
  const char* kFreedesktopPowermanagerInterface
      = "org.freedesktop.PowerManagement.Inhibit";
}
