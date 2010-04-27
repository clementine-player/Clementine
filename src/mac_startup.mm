#import <IOKit/hidsystem/ev_keymap.h>
#import <AppKit/NSEvent.h>

#import <AppKit/NSApplication.h>

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSNibDeclarations.h>
#import <Sparkle/SUUpdater.h>

#include "globalshortcuts/globalshortcuts.h"
#include "mac_startup.h"

#include <QCoreApplication>
#include <QEvent>
#include <QObject>

// Capture global media keys on Mac (Cocoa only!)
// See: http://www.rogueamoeba.com/utm/2007/09/29/apple-keyboard-media-key-event-handling/

@interface MacApplication :NSApplication {
  GlobalShortcuts* shortcut_handler_;
  QObject* application_handler_;
}

- (GlobalShortcuts*) shortcut_handler;
- (void) SetShortcutHandler: (GlobalShortcuts*)handler;

- (QObject*) application_handler;
- (void) SetApplicationHandler: (QObject*)handler;

- (void) mediaKeyEvent: (int)key state: (BOOL)state repeat: (BOOL)repeat;
@end

@interface AppDelegate :NSObject <NSApplicationDelegate> {
  QObject* application_handler_;
}

- (id) initWithHandler: (QObject*)handler;
// NSApplicationDelegate
- (BOOL) applicationShouldHandleReopen: (NSApplication*)app hasVisibleWindows:(BOOL)flag;
@end

@implementation AppDelegate

- (id) init {
  if ((self = [super init])) {
    application_handler_ = nil;
  }
  return self;
}

- (id) initWithHandler: (QObject*)handler {
  application_handler_ = handler;
  return self;
}

- (BOOL) applicationShouldHandleReopen: (NSApplication*)app hasVisibleWindows:(BOOL)flag {
  if (application_handler_) {
    qApp->postEvent(application_handler_, new QEvent(QEvent::ApplicationActivate));
  }
  return YES;
}
@end

@implementation MacApplication

- (id) init {
  if ((self = [super init])) {
    [self SetShortcutHandler:nil];
    [self SetApplicationHandler:nil];
  }
  return self;
}

- (GlobalShortcuts*) shortcut_handler {
  return shortcut_handler_;
}

- (void) SetShortcutHandler: (GlobalShortcuts*)handler {
  shortcut_handler_ = handler;
}

- (QObject*) application_handler {
  return application_handler_;
}

- (void) SetApplicationHandler: (QObject*)handler {
  AppDelegate* delegate = [[AppDelegate alloc] initWithHandler:handler];
  [self setDelegate:delegate];
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
  if (!shortcut_handler_) {
    return;
  }
  if (state == 0) {
    switch (key) {
      case NX_KEYTYPE_PLAY:
        // Play pressed.
        shortcut_handler_->MacMediaKeyPressed("Play");
        break;
      case NX_KEYTYPE_FAST:
        // Next pressed.
        shortcut_handler_->MacMediaKeyPressed("Next");
        break;
      case NX_KEYTYPE_REWIND:
        shortcut_handler_->MacMediaKeyPressed("Previous");
        break;
      default:
        break;
    }
  }
}

@end

namespace mac {

void MacMain() {
  [[NSAutoreleasePool alloc] init];
  // Creates and sets the magic global variable so QApplication will find it.
  [MacApplication sharedApplication];
  // Creates and sets the magic global variable for Sparkle.
  [[SUUpdater sharedUpdater] setDelegate: NSApp];
}

void SetShortcutHandler(GlobalShortcuts* handler) {
  [NSApp SetShortcutHandler: handler];
}

void SetApplicationHandler(QObject* handler) {
  [NSApp SetApplicationHandler: handler];
}

void CheckForUpdates() {
  [[SUUpdater sharedUpdater] checkForUpdates: NSApp];
}

}  // namespace mac
