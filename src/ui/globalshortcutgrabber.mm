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

#include <boost/noncopyable.hpp>

#import "core/mac_utilities.h"

class MacMonitorWrapper : boost::noncopyable {
 public:
  explicit MacMonitorWrapper(id monitor) : local_monitor_(monitor) {}

  ~MacMonitorWrapper() { [NSEvent removeMonitor:local_monitor_]; }

 private:
  id local_monitor_;
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
  id monitor =
      [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask
                                            handler:^(NSEvent* event) {
                                                return HandleMacEvent(event)
                                                           ? event
                                                           : nil;
                                            }];
  wrapper_ = new MacMonitorWrapper(monitor);
}

void GlobalShortcutGrabber::TeardownMacEventHandler() { delete wrapper_; }
