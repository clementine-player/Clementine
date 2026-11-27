/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "globalshortcutgrabber.h"

#import <AppKit/NSEvent.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSViewController.h>
#import <QuartzCore/CALayer.h>

#include <QKeySequence>

#import "core/mac_utilities.h"

class MacMonitorWrapper {
 public:
  // Under MRC, retain the autoreleased monitor returned by NSEvent so it
  // outlives the current autorelease pool drain until removed in ~Wrapper.
  explicit MacMonitorWrapper(id monitor) : local_monitor_([monitor retain]) {}

  ~MacMonitorWrapper() {
    if (local_monitor_) {
      [NSEvent removeMonitor:local_monitor_];
      [local_monitor_ release];
    }
  }

 private:
  id local_monitor_;
  Q_DISABLE_COPY(MacMonitorWrapper);
};

bool GlobalShortcutGrabber::HandleMacEvent(NSEvent* event) {
  ret_ = mac::KeySequenceFromNSEvent(event);
  UpdateText();
  if ([[event charactersIgnoringModifiers] length] != 0) {
    accept();
    return true;
  }
  return ret_ == QKeySequence(Qt::Key_Escape);
}

void GlobalShortcutGrabber::SetupMacEventHandler() {
  // Tear down any existing monitor first in case showEvent() fires twice.
  TeardownMacEventHandler();
  id monitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask
                                                     handler:^(NSEvent* event) {
                                                       return HandleMacEvent(event) ? event : nil;
                                                     }];
  wrapper_ = new MacMonitorWrapper(monitor);
}

void GlobalShortcutGrabber::TeardownMacEventHandler() {
  // Clear wrapper_ before deleting to prevent double-free on re-entry.
  MacMonitorWrapper* wrapper = wrapper_;
  wrapper_ = nullptr;
  delete wrapper;
}
