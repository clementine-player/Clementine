#import <AppKit/NSApplication.h>

#import <Breakpad/Breakpad.h>

#include "config.h"

class PlatformInterface;

#ifdef SNOW_LEOPARD
@interface AppDelegate :NSObject <NSApplicationDelegate> {
#else
@interface AppDelegate :NSObject {
#endif
  PlatformInterface* application_handler_;
  NSMenu* dock_menu_;

  BreakpadRef breakpad_;
}

- (id) initWithHandler: (PlatformInterface*)handler;
// NSApplicationDelegate
- (BOOL) applicationShouldHandleReopen: (NSApplication*)app hasVisibleWindows:(BOOL)flag;
- (NSMenu*) applicationDockMenu: (NSApplication*)sender;
- (void) setDockMenu: (NSMenu*)menu;

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication*)sender;
@end

