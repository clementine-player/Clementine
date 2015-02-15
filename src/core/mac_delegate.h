/* This file is part of Clementine.
   Copyright 2011, David Sansome <davidsansome@gmail.com>
   Copyright 2011-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#import <AppKit/NSApplication.h>

#include "config.h"
#include "macglobalshortcutbackend.h"

#ifdef HAVE_BREAKPAD
#import <Breakpad/Breakpad.h>
#endif

class PlatformInterface;
@class SPMediaKeyTap;

@interface AppDelegate : NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate> {
  PlatformInterface* application_handler_;
  NSMenu* dock_menu_;
  MacGlobalShortcutBackend* shortcut_handler_;
  SPMediaKeyTap* key_tap_;

#ifdef HAVE_BREAKPAD
  BreakpadRef breakpad_;
#endif
}

- (id)initWithHandler:(PlatformInterface*)handler;

// NSApplicationDelegate
- (BOOL)applicationShouldHandleReopen:(NSApplication*)app
                    hasVisibleWindows:(BOOL)flag;
- (NSMenu*)applicationDockMenu:(NSApplication*)sender;
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;
- (NSApplicationTerminateReply)applicationShouldTerminate:
        (NSApplication*)sender;

// NSUserNotificationCenterDelegate
- (BOOL)userNotificationCenter:(id)center
     shouldPresentNotification:(id)notification;

- (void)setDockMenu:(NSMenu*)menu;
- (MacGlobalShortcutBackend*)shortcut_handler;
- (void)setShortcutHandler:(MacGlobalShortcutBackend*)backend;
- (void)mediaKeyTap:(SPMediaKeyTap*)keyTap
    receivedMediaKeyEvent:(NSEvent*)event;
@end
