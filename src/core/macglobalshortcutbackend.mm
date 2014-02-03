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

#include "macglobalshortcutbackend.h"

#include "config.h"
#include "globalshortcuts.h"
#include "mac_startup.h"
#import "mac_utilities.h"

#include <boost/noncopyable.hpp>

#include <QAction>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QtDebug>

#include <AppKit/NSEvent.h>
#include <AppKit/NSWorkspace.h>
#include <Foundation/NSString.h>
#include <IOKit/hidsystem/ev_keymap.h>

class MacGlobalShortcutBackendPrivate : boost::noncopyable {
 public:
  explicit MacGlobalShortcutBackendPrivate(MacGlobalShortcutBackend* backend)
      : global_monitor_(nil), local_monitor_(nil), backend_(backend) {}

  bool Register() {
    global_monitor_ =
        [NSEvent addGlobalMonitorForEventsMatchingMask:NSKeyDownMask
                                               handler:^(NSEvent* event) {
                                                   HandleKeyEvent(event);
                                               }];
    local_monitor_ =
        [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask
                                              handler:^(NSEvent* event) {
                                                  // Filter event if we handle
                                                  // it as a global shortcut.
                                                  return HandleKeyEvent(event)
                                                             ? nil
                                                             : event;
                                              }];
    return true;
  }

  void Unregister() {
    [NSEvent removeMonitor:global_monitor_];
    [NSEvent removeMonitor:local_monitor_];
  }

 private:
  bool HandleKeyEvent(NSEvent* event) {
    QKeySequence sequence = mac::KeySequenceFromNSEvent(event);
    return backend_->KeyPressed(sequence);
  }

  id global_monitor_;
  id local_monitor_;
  MacGlobalShortcutBackend* backend_;
};

MacGlobalShortcutBackend::MacGlobalShortcutBackend(GlobalShortcuts* parent)
    : GlobalShortcutBackend(parent),
      p_(new MacGlobalShortcutBackendPrivate(this)) {}

MacGlobalShortcutBackend::~MacGlobalShortcutBackend() {}

bool MacGlobalShortcutBackend::DoRegister() {
  // Always enable media keys.
  mac::SetShortcutHandler(this);

  if (AXAPIEnabled()) {
    for (const GlobalShortcuts::Shortcut& shortcut :
         manager_->shortcuts().values()) {
      shortcuts_[shortcut.action->shortcut()] = shortcut.action;
    }
    return p_->Register();
  }

  return false;
}

void MacGlobalShortcutBackend::DoUnregister() {
  p_->Unregister();
  shortcuts_.clear();
}

void MacGlobalShortcutBackend::MacMediaKeyPressed(int key) {
  switch (key) {
    case NX_KEYTYPE_PLAY:
      KeyPressed(Qt::Key_MediaPlay);
      break;
    case NX_KEYTYPE_FAST:
      KeyPressed(Qt::Key_MediaNext);
      break;
    case NX_KEYTYPE_REWIND:
      KeyPressed(Qt::Key_MediaPrevious);
      break;
  }
}

bool MacGlobalShortcutBackend::KeyPressed(const QKeySequence& sequence) {
  if (sequence.isEmpty()) {
    return false;
  }
  QAction* action = shortcuts_[sequence];
  if (action) {
    action->trigger();
    return true;
  }
  return false;
}

bool MacGlobalShortcutBackend::IsAccessibilityEnabled() const {
  return AXAPIEnabled();
}

void MacGlobalShortcutBackend::ShowAccessibilityDialog() {
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSPreferencePanesDirectory, NSSystemDomainMask, YES);
  if ([paths count] == 1) {
    NSURL* prefpane_url =
        [NSURL fileURLWithPath:[[paths objectAtIndex:0]
                                   stringByAppendingPathComponent:
                                       @"UniversalAccessPref.prefPane"]];
    [[NSWorkspace sharedWorkspace] openURL:prefpane_url];
  }
}
