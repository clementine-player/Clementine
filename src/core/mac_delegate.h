#import <AppKit/NSApplication.h>

#include "config.h"

#ifdef HAVE_BREAKPAD
#import <Breakpad/Breakpad.h>
#endif


class PlatformInterface;

#ifdef SNOW_LEOPARD
@interface AppDelegate :NSObject <NSApplicationDelegate> {
#else
@interface AppDelegate :NSObject {
#endif
  PlatformInterface* application_handler_;
  NSMenu* dock_menu_;

#ifdef HAVE_BREAKPAD
  BreakpadRef breakpad_;
#endif
}

- (id) initWithHandler: (PlatformInterface*)handler;
// NSApplicationDelegate
- (BOOL) applicationShouldHandleReopen: (NSApplication*)app hasVisibleWindows:(BOOL)flag;
- (NSMenu*) applicationDockMenu: (NSApplication*)sender;
- (void) setDockMenu: (NSMenu*)menu;

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication*)sender;
@end

