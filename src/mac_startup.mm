#import <IOKit/hidsystem/ev_keymap.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>

#include "globalshortcuts/globalshortcuts.h"
#include "mac_startup.h"

// Capture global media keys on Mac (Cocoa only!)
// See: http://www.rogueamoeba.com/utm/2007/09/29/apple-keyboard-media-key-event-handling/

@interface MacApplication :NSApplication {
  GlobalShortcuts* handler_;
}

- (GlobalShortcuts*) handler;
- (void) SetHandler: (GlobalShortcuts*)handler;

-(void) mediaKeyEvent: (int)key state: (BOOL)state repeat: (BOOL)repeat;
@end

@implementation MacApplication

- (id) init {
  if ((self = [super init])) {
    [self SetHandler:nil];
  }
  return self;
}

- (GlobalShortcuts*) handler {
  return handler_;
}

- (void) SetHandler: (GlobalShortcuts*)handler {
  handler_ = handler;
}

-(void) sendEvent: (NSEvent*)event {
  if ([event type] == NSSystemDefined && [event subtype] == 8) {
    int keycode = (([event data1] & 0xFFFF0000) >> 16);
    int keyflags = ([event data1] & 0x0000FFFF);
    int keystate = (((keyflags & 0xFF00) >> 8)) == 0xA;
    int keyrepeat = (keyflags & 0x1);

    [self mediaKeyEvent: keycode state: keystate repeat: keyrepeat];
  }

  [super sendEvent: event];
}

-(void) mediaKeyEvent: (int)key state: (BOOL)state repeat: (BOOL)repeat {
  if (!handler_) {
    return;
  }
  if (state == 0) {
    switch (key) {
      case NX_KEYTYPE_PLAY:
        // Play pressed.
        handler_->MacMediaKeyPressed("Play");
        break;
      case NX_KEYTYPE_FAST:
        // Next pressed.
        handler_->MacMediaKeyPressed("Next");
        break;
      case NX_KEYTYPE_REWIND:
        handler_->MacMediaKeyPressed("Previous");
        break;
      default:
        break;
    }
  }
}

@end

namespace mac {

void MacMain() {
  // Creates and sets the magic global variable so QApplication will find it.
  [MacApplication sharedApplication];
}

void SetShortcutHandler(GlobalShortcuts* handler) {
  [NSApp SetHandler: handler];
}

}  // namespace mac
