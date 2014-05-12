#import <AppKit/NSApplication.h>

#include "config.h"
#include "macglobalshortcutbackend.h"

#ifdef HAVE_BREAKPAD
#import <Breakpad/Breakpad.h>
#endif

class PlatformInterface;
@class SPMediaKeyTap;

@interface AppDelegate : NSObject<NSApplicationDelegate> {
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
