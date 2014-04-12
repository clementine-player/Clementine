/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include <QtDebug>

#include "core/utilities.h"

// kIOPMAssertionTypePreventUserIdleDisplaySleep from Lion.
#define kLionDisplayAssertion CFSTR("PreventUserIdleDisplaySleep")

MacScreensaver::MacScreensaver() : assertion_id_(0) {}

void MacScreensaver::Inhibit() {
  CFStringRef assertion_type = (Utilities::GetMacVersion() >= 7)
                                   ? kLionDisplayAssertion
                                   : kIOPMAssertionTypeNoDisplaySleep;

  IOPMAssertionCreateWithName(
      assertion_type, kIOPMAssertionLevelOn,
      CFSTR("Showing full-screen Clementine visualisations"), &assertion_id_);
}

void MacScreensaver::Uninhibit() { IOPMAssertionRelease(assertion_id_); }
