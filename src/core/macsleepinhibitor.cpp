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

#include "core/macsleepinhibitor.h"

#include "core/logging.h"

MacSleepInhibitor::~MacSleepInhibitor() {
  for (IOPMAssertionID assertion : assertions_) {
    IOPMAssertionRelease(assertion);
  }
}

void MacSleepInhibitor::Inhibit(SleepInhibitor::Keep keep,
                                const QString& reason) {
  CFStringRef type = keep == SleepInhibitor::Keep::DisplayOn
                         ? kIOPMAssertPreventUserIdleDisplaySleep
                         : kIOPMAssertPreventUserIdleSystemSleep;
  CFStringRef name = reason.toCFString();
  IOPMAssertionID assertion = 0;
  IOReturn result = IOPMAssertionCreateWithName(type, kIOPMAssertionLevelOn,
                                                name, &assertion);
  CFRelease(name);
  if (result != kIOReturnSuccess) {
    qLog(Warning) << "Couldn't keep the computer awake:" << result;
    return;
  }
  assertions_[keep] = assertion;
}

void MacSleepInhibitor::Uninhibit(SleepInhibitor::Keep keep) {
  if (!assertions_.contains(keep)) return;
  IOPMAssertionRelease(assertions_.take(keep));
}
