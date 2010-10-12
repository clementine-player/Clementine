/* This file is part of Clementine.

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

#include "macscreensaver.h"

#include <CoreServices/CoreServices.h>

#include <QtDebug>

MacScreensaver::MacScreensaver() {
  timer_.setInterval(30000);
  connect(&timer_, SIGNAL(timeout()), SLOT(Timeout()));
}

void MacScreensaver::Timeout() {
  UpdateSystemActivity(OverallAct);
}

void MacScreensaver::Inhibit() {
  timer_.start(30000);
  Timeout();
}

void MacScreensaver::Uninhibit() {
  timer_.stop();
}
