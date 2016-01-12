/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2015 - 2016, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

#include "macidlehandler.h"

#include <IOKit/pwr_mgt/IOPMLib.h>

#include <QtDebug>

#include "core/logging.h"
#include "core/utilities.h"

MacIdleHandler::MacIdleHandler() : assertion_id_(0), 
                                   is_inhibit_(false) {}

void MacIdleHandler::Inhibit(const char* reason) {
  CFStringRef cfstr;
  IOReturn reply;
  // NOTE: this does not create a memmory leak
  char* buffer = (char*)malloc(strlen(reason) + 1);
  strcpy(buffer, reason);

  cfstr = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, buffer,
                                          kCFStringEncodingUTF8,
                                          kCFAllocatorDefault);
  switch (IdleHandler::inbtr_) {
    case IdleHandler::Screensaver :
      // resetting the display idle timer.
      reply = IOPMAssertionCreateWithName(
          kIOPMAssertPreventUserIdleDisplaySleep, kIOPMAssertionLevelOn,
          cfstr, &assertion_id_);
      break;
    case IdleHandler::Suspend :
      // resetting the system idle timer.
      reply = IOPMAssertionCreateWithName(
          kIOPMAssertPreventUserIdleSystemSleep, kIOPMAssertionLevelOn,
          cfstr, &assertion_id_);
      break;
  }
  CFRelease(cfstr);

  if (reply == kIOReturnSuccess) {
    is_inhibit_ = true;
  } else {
    qLog(Warning) << "Failed to inhibit screensaver/suspend";
  }
}

void MacIdleHandler::Uninhibit() {
  IOReturn reply = IOPMAssertionRelease(assertion_id_);

  if (reply == kIOReturnSuccess) {
    is_inhibit_ = false;
  } else {
    qLog(Warning) << "Failed to uninhibit screensaver/suspend";
  }
}

bool MacIdleHandler::Isinhibited() {
  return is_inhibit_;
}
