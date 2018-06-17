/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>

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
#import <AppKit/NSEvent.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSViewController.h>

#import <Foundation/NSBundle.h>
#import <Foundation/NSError.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSURL.h>

#import <IOKit/hidsystem/ev_keymap.h>

#import <Kernel/AvailabilityMacros.h>

#import <QuartzCore/CALayer.h>

#import "3rdparty/SPMediaKeyTap/SPMediaKeyTap.h"

#include "config.h"
#include "globalshortcuts.h"
#include "mac_delegate.h"
#include "mac_startup.h"
#include "mac_utilities.h"
#include "macglobalshortcutbackend.h"
#include "utilities.h"
#include "core/logging.h"
#include "core/scoped_cftyperef.h"
#include "core/scoped_nsautorelease_pool.h"

#ifdef HAVE_SPARKLE
#import <Sparkle/SUUpdater.h>
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QSettings>
#include <QWidget>

#include <QtDebug>

QDebug operator<<(QDebug dbg, NSObject* object) {
  QString ns_format = [[NSString stringWithFormat:@"%@", object] UTF8String];
  dbg.nospace() << ns_format;
  return dbg.space();
}

// Capture global media keys on Mac (Cocoa only!)
// See:
// http://www.rogueamoeba.com/utm/2007/09/29/apple-keyboard-media-key-event-handling/

@interface MacApplication : NSApplication {
  PlatformInterface* application_handler_;
  AppDelegate* delegate_;
  // shortcut_handler_ only used to temporarily save it
  // AppDelegate does all the heavy-shortcut-lifting
  MacGlobalShortcutBackend* shortcut_handler_;
}

- (MacGlobalShortcutBackend*)shortcut_handler;
- (void)SetShortcutHandler:(MacGlobalShortcutBackend*)handler;

- (PlatformInterface*)application_handler;
- (void)SetApplicationHandler:(PlatformInterface*)handler;

@end

#ifdef HAVE_BREAKPAD
static bool BreakpadCallback(int, int, mach_port_t, void*) { return true; }

static BreakpadRef InitBreakpad() {
  ScopedNSAutoreleasePool pool;
  BreakpadRef breakpad = nil;
  NSDictionary* plist = [[NSBundle mainBundle] infoDictionary];
  if (plist) {
    breakpad = BreakpadCreate(plist);
    BreakpadSetFilterCallback(breakpad, &BreakpadCallback, nullptr);
  }
  [pool release];
  return breakpad;
}
#endif  // HAVE_BREAKPAD

@implementation AppDelegate

- (id)init {
  if ((self = [super init])) {
    application_handler_ = nil;
    shortcut_handler_ = nil;
    dock_menu_ = nil;
  }
  return self;
}

- (id)initWithHandler:(PlatformInterface*)handler {
  application_handler_ = handler;

#ifdef HAVE_BREAKPAD
  breakpad_ = InitBreakpad();
#endif

  // Register defaults for the whitelist of apps that want to use media keys
  [[NSUserDefaults standardUserDefaults]
      registerDefaults:
          [NSDictionary
              dictionaryWithObjectsAndKeys:
                  [SPMediaKeyTap defaultMediaKeyUserBundleIdentifiers],
                  kMediaKeyUsingBundleIdentifiersDefaultsKey, nil]];
  return self;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)app
                    hasVisibleWindows:(BOOL)flag {
  if (application_handler_) {
    application_handler_->Activate();
  }
  return YES;
}

- (void)setDockMenu:(NSMenu*)menu {
  dock_menu_ = menu;
}

- (NSMenu*)applicationDockMenu:(NSApplication*)sender {
  return dock_menu_;
}

- (void)setShortcutHandler:(MacGlobalShortcutBackend*)backend {
  shortcut_handler_ = backend;
}

- (MacGlobalShortcutBackend*)shortcut_handler {
  return shortcut_handler_;
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
  key_tap_ = [[SPMediaKeyTap alloc] initWithDelegate:self];
  if ([SPMediaKeyTap usesGlobalMediaKeyTap] &&
      ![[NSProcessInfo processInfo]
          isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){
                                              .majorVersion = 10,
                                              .minorVersion = 12,
                                              .patchVersion = 0}]) {
    [key_tap_ startWatchingMediaKeys];
  } else {
    qLog(Warning) << "Media key monitoring disabled";
  }
}

- (BOOL)application:(NSApplication*)app openFile:(NSString*)filename {
  qLog(Debug) << "Wants to open:" << [filename UTF8String];

  if (application_handler_->LoadUrl(QString::fromUtf8([filename UTF8String]))) {
    return YES;
  }

  return NO;
}

- (void)application:(NSApplication*)app openFiles:(NSArray*)filenames {
  qLog(Debug) << "Wants to open:" << filenames;
  [filenames
      enumerateObjectsUsingBlock:^(id object, NSUInteger idx, BOOL* stop) {
          [self application:app openFile:(NSString*)object];
      }];
}

- (void)mediaKeyTap:(SPMediaKeyTap*)keyTap
    receivedMediaKeyEvent:(NSEvent*)event {
  NSAssert([event type] == NSSystemDefined &&
               [event subtype] == SPSystemDefinedEventMediaKeys,
           @"Unexpected NSEvent in mediaKeyTap:receivedMediaKeyEvent:");

  int key_code = (([event data1] & 0xFFFF0000) >> 16);
  int key_flags = ([event data1] & 0x0000FFFF);
  BOOL key_is_released = (((key_flags & 0xFF00) >> 8)) == 0xB;
  // not used. keep just in case
  //  int key_repeat = (key_flags & 0x1);

  if (!shortcut_handler_) {
    return;
  }
  if (key_is_released) {
    shortcut_handler_->MacMediaKeyPressed(key_code);
  }
}

- (NSApplicationTerminateReply)applicationShouldTerminate:
                                   (NSApplication*)sender {
#ifdef HAVE_BREAKPAD
  BreakpadRelease(breakpad_);
#endif
  return NSTerminateNow;
}

- (BOOL)userNotificationCenter:(id)center
     shouldPresentNotification:(id)notification {
  // Always show notifications, even if Clementine is in the foreground.
  return YES;
}

@end

@implementation MacApplication

- (id)init {
  if ((self = [super init])) {
    [self SetShortcutHandler:nil];
  }
  return self;
}

- (MacGlobalShortcutBackend*)shortcut_handler {
  // should be the same as delegate_'s shortcut handler
  return shortcut_handler_;
}

- (void)SetShortcutHandler:(MacGlobalShortcutBackend*)handler {
  shortcut_handler_ = handler;
  if (delegate_) [delegate_ setShortcutHandler:handler];
}

- (PlatformInterface*)application_handler {
  return application_handler_;
}

- (void)SetApplicationHandler:(PlatformInterface*)handler {
  delegate_ = [[AppDelegate alloc] initWithHandler:handler];
  // App-shortcut-handler set before delegate is set.
  // this makes sure the delegate's shortcut_handler is set
  [delegate_ setShortcutHandler:shortcut_handler_];
  [self setDelegate:delegate_];

  [[NSUserNotificationCenter defaultUserNotificationCenter]
      setDelegate:delegate_];
}

- (void)sendEvent:(NSEvent*)event {
  // If event tap is not installed, handle events that reach the app instead
  BOOL shouldHandleMediaKeyEventLocally =
      ![SPMediaKeyTap usesGlobalMediaKeyTap];

  if (shouldHandleMediaKeyEventLocally && [event type] == NSSystemDefined &&
      [event subtype] == SPSystemDefinedEventMediaKeys) {
    [(id)[self delegate] mediaKeyTap:nil receivedMediaKeyEvent:event];
  }

  [super sendEvent:event];
}

@end

namespace mac {

void MacMain() {
  ScopedNSAutoreleasePool pool;
  // Creates and sets the magic global variable so QApplication will find it.
  [MacApplication sharedApplication];
#ifdef HAVE_SPARKLE
  // Creates and sets the magic global variable for Sparkle.
  [[SUUpdater sharedUpdater] setDelegate:NSApp];
#endif
}

void SetShortcutHandler(MacGlobalShortcutBackend* handler) {
  [NSApp SetShortcutHandler:handler];
}

void SetApplicationHandler(PlatformInterface* handler) {
  [NSApp SetApplicationHandler:handler];
}

void CheckForUpdates() {
#ifdef HAVE_SPARKLE
  [[SUUpdater sharedUpdater] checkForUpdates:NSApp];
#endif
}

QString GetBundlePath() {
  ScopedCFTypeRef<CFURLRef> app_url(
      CFBundleCopyBundleURL(CFBundleGetMainBundle()));
  ScopedCFTypeRef<CFStringRef> mac_path(
      CFURLCopyFileSystemPath(app_url.get(), kCFURLPOSIXPathStyle));
  const char* path =
      CFStringGetCStringPtr(mac_path.get(), CFStringGetSystemEncoding());
  QString bundle_path = QString::fromUtf8(path);
  return bundle_path;
}

QString GetResourcesPath() {
  QString bundle_path = GetBundlePath();
  return bundle_path + "/Contents/Resources";
}

QString GetApplicationSupportPath() {
  ScopedNSAutoreleasePool pool;
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES);
  QString ret;
  if ([paths count] > 0) {
    NSString* user_path = [paths objectAtIndex:0];
    ret = QString::fromUtf8([user_path UTF8String]);
  } else {
    ret = "~/Library/Application Support";
  }
  return ret;
}

QString GetMusicDirectory() {
  ScopedNSAutoreleasePool pool;
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSMusicDirectory,
                                                       NSUserDomainMask, YES);
  QString ret;
  if ([paths count] > 0) {
    NSString* user_path = [paths objectAtIndex:0];
    ret = QString::fromUtf8([user_path UTF8String]);
  } else {
    ret = "~/Music";
  }
  return ret;
}

static int MapFunctionKey(int keycode) {
  switch (keycode) {
    // Function keys
    case NSInsertFunctionKey:
      return Qt::Key_Insert;
    case NSDeleteFunctionKey:
      return Qt::Key_Delete;
    case NSPauseFunctionKey:
      return Qt::Key_Pause;
    case NSPrintFunctionKey:
      return Qt::Key_Print;
    case NSSysReqFunctionKey:
      return Qt::Key_SysReq;
    case NSHomeFunctionKey:
      return Qt::Key_Home;
    case NSEndFunctionKey:
      return Qt::Key_End;
    case NSLeftArrowFunctionKey:
      return Qt::Key_Left;
    case NSUpArrowFunctionKey:
      return Qt::Key_Up;
    case NSRightArrowFunctionKey:
      return Qt::Key_Right;
    case NSDownArrowFunctionKey:
      return Qt::Key_Down;
    case NSPageUpFunctionKey:
      return Qt::Key_PageUp;
    case NSPageDownFunctionKey:
      return Qt::Key_PageDown;
    case NSScrollLockFunctionKey:
      return Qt::Key_ScrollLock;
    case NSF1FunctionKey:
      return Qt::Key_F1;
    case NSF2FunctionKey:
      return Qt::Key_F2;
    case NSF3FunctionKey:
      return Qt::Key_F3;
    case NSF4FunctionKey:
      return Qt::Key_F4;
    case NSF5FunctionKey:
      return Qt::Key_F5;
    case NSF6FunctionKey:
      return Qt::Key_F6;
    case NSF7FunctionKey:
      return Qt::Key_F7;
    case NSF8FunctionKey:
      return Qt::Key_F8;
    case NSF9FunctionKey:
      return Qt::Key_F9;
    case NSF10FunctionKey:
      return Qt::Key_F10;
    case NSF11FunctionKey:
      return Qt::Key_F11;
    case NSF12FunctionKey:
      return Qt::Key_F12;
    case NSF13FunctionKey:
      return Qt::Key_F13;
    case NSF14FunctionKey:
      return Qt::Key_F14;
    case NSF15FunctionKey:
      return Qt::Key_F15;
    case NSF16FunctionKey:
      return Qt::Key_F16;
    case NSF17FunctionKey:
      return Qt::Key_F17;
    case NSF18FunctionKey:
      return Qt::Key_F18;
    case NSF19FunctionKey:
      return Qt::Key_F19;
    case NSF20FunctionKey:
      return Qt::Key_F20;
    case NSF21FunctionKey:
      return Qt::Key_F21;
    case NSF22FunctionKey:
      return Qt::Key_F22;
    case NSF23FunctionKey:
      return Qt::Key_F23;
    case NSF24FunctionKey:
      return Qt::Key_F24;
    case NSF25FunctionKey:
      return Qt::Key_F25;
    case NSF26FunctionKey:
      return Qt::Key_F26;
    case NSF27FunctionKey:
      return Qt::Key_F27;
    case NSF28FunctionKey:
      return Qt::Key_F28;
    case NSF29FunctionKey:
      return Qt::Key_F29;
    case NSF30FunctionKey:
      return Qt::Key_F30;
    case NSF31FunctionKey:
      return Qt::Key_F31;
    case NSF32FunctionKey:
      return Qt::Key_F32;
    case NSF33FunctionKey:
      return Qt::Key_F33;
    case NSF34FunctionKey:
      return Qt::Key_F34;
    case NSF35FunctionKey:
      return Qt::Key_F35;
    case NSMenuFunctionKey:
      return Qt::Key_Menu;
    case NSHelpFunctionKey:
      return Qt::Key_Help;
  }

  return 0;
}

QKeySequence KeySequenceFromNSEvent(NSEvent* event) {
  NSString* str = [event charactersIgnoringModifiers];
  NSString* upper = [str uppercaseString];
  const char* chars = [upper UTF8String];
  NSUInteger modifiers = [event modifierFlags];
  int key = 0;
  unsigned char c = chars[0];
  switch (c) {
    case 0x1b:
      key = Qt::Key_Escape;
      break;
    case 0x09:
      key = Qt::Key_Tab;
      break;
    case 0x0d:
      key = Qt::Key_Return;
      break;
    case 0x08:
      key = Qt::Key_Backspace;
      break;
    case 0x03:
      key = Qt::Key_Enter;
      break;
  }

  if (key == 0) {
    if (c >= 0x20 && c <= 0x7e) {  // ASCII from space to ~
      key = c;
    } else {
      key = MapFunctionKey([event keyCode]);
      if (key == 0) {
        return QKeySequence();
      }
    }
  }

  if (modifiers & NSShiftKeyMask) {
    key += Qt::SHIFT;
  }
  if (modifiers & NSControlKeyMask) {
    key += Qt::META;
  }
  if (modifiers & NSAlternateKeyMask) {
    key += Qt::ALT;
  }
  if (modifiers & NSCommandKeyMask) {
    key += Qt::CTRL;
  }

  return QKeySequence(key);
}

void DumpDictionary(CFDictionaryRef dict) {
  NSDictionary* d = (NSDictionary*)dict;
  NSLog(@"%@", d);
}

// NSWindowCollectionBehaviorFullScreenPrimary
static const NSUInteger kFullScreenPrimary = 1 << 7;

void EnableFullScreen(const QWidget& main_window) {
  NSView* view = reinterpret_cast<NSView*>(main_window.winId());
  NSWindow* window = [view window];
  [window setCollectionBehavior:kFullScreenPrimary];
}

float GetDevicePixelRatio(QWidget* widget) {
  NSView* view = reinterpret_cast<NSView*>(widget->winId());
  return [[view window] backingScaleFactor];
}

}  // namespace mac
