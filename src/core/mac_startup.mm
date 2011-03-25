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

#import <IOKit/hidsystem/ev_keymap.h>
#import <AppKit/NSEvent.h>

#import <AppKit/NSApplication.h>

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSError.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSNibDeclarations.h>

#import <Kernel/AvailabilityMacros.h>

#include "config.h"
#include "globalshortcuts.h"
#include "mac_delegate.h"
#include "mac_startup.h"
#include "macglobalshortcutbackend.h"
#include "utilities.h"

#ifdef HAVE_SPARKLE
#import <Sparkle/SUUpdater.h>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QSettings>

#include <QtDebug>

// Capture global media keys on Mac (Cocoa only!)
// See: http://www.rogueamoeba.com/utm/2007/09/29/apple-keyboard-media-key-event-handling/

@interface MacApplication :NSApplication {
  MacGlobalShortcutBackend* shortcut_handler_;
  PlatformInterface* application_handler_;
}

- (MacGlobalShortcutBackend*) shortcut_handler;
- (void) SetShortcutHandler: (MacGlobalShortcutBackend*)handler;

- (PlatformInterface*) application_handler;
- (void) SetApplicationHandler: (PlatformInterface*)handler;

- (void) mediaKeyEvent: (int)key state: (BOOL)state repeat: (BOOL)repeat;
@end

static bool BreakpadCallback(int, int, mach_port_t, void*) {
  return true;
}

static BreakpadRef InitBreakpad() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  BreakpadRef breakpad = nil;
  NSDictionary* plist = [[NSBundle mainBundle] infoDictionary];
  if (plist) {
    breakpad = BreakpadCreate(plist);
    BreakpadSetFilterCallback(breakpad, &BreakpadCallback, NULL);
  }
  [pool release];
  return breakpad;
}

@implementation AppDelegate

- (id) init {
  if ((self = [super init])) {
    application_handler_ = nil;
    dock_menu_ = nil;
  }
  return self;
}

- (id) initWithHandler: (PlatformInterface*)handler {
  application_handler_ = handler;
  breakpad_ = InitBreakpad();
  return self;
}

- (BOOL) applicationShouldHandleReopen: (NSApplication*)app hasVisibleWindows:(BOOL)flag {
  if (application_handler_) {
    application_handler_->Activate();
  }
  return YES;
}

- (void) setDockMenu: (NSMenu*)menu {
  dock_menu_ = menu;
}

- (NSMenu*) applicationDockMenu: (NSApplication*)sender {
  return dock_menu_;
}

- (BOOL) application: (NSApplication*)app openFile:(NSString*)filename {
  qDebug() << "Wants to open:" << [filename UTF8String];

  if (application_handler_->LoadUrl(QString::fromUtf8([filename UTF8String]))) {
    return YES;
  }

  return NO;
}

- (NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication*) sender {
  BreakpadRelease(breakpad_);
  return NSTerminateNow;
}
@end

@implementation MacApplication

- (id) init {
  if ((self = [super init])) {
    [self SetShortcutHandler:nil];
  }
  return self;
}

- (MacGlobalShortcutBackend*) shortcut_handler {
  return shortcut_handler_;
}

- (void) SetShortcutHandler: (MacGlobalShortcutBackend*)handler {
  shortcut_handler_ = handler;
}

- (PlatformInterface*) application_handler {
  return application_handler_;
}

- (void) SetApplicationHandler: (PlatformInterface*)handler {
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
    shortcut_handler_->MacMediaKeyPressed(key);
  }
}

@end

namespace mac {

void MacMain() {
  [[NSAutoreleasePool alloc] init];
  // Creates and sets the magic global variable so QApplication will find it.
  [MacApplication sharedApplication];
  #ifdef HAVE_SPARKLE
    // Creates and sets the magic global variable for Sparkle.
    [[SUUpdater sharedUpdater] setDelegate: NSApp];
  #endif
}

void SetShortcutHandler(MacGlobalShortcutBackend* handler) {
  [NSApp SetShortcutHandler: handler];
}

void SetApplicationHandler(PlatformInterface* handler) {
  [NSApp SetApplicationHandler: handler];
}

void CheckForUpdates() {
  #ifdef HAVE_SPARKLE
  [[SUUpdater sharedUpdater] checkForUpdates: NSApp];
  #endif
}

QString GetBundlePath() {
  CFURLRef app_url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFStringRef mac_path = CFURLCopyFileSystemPath(app_url, kCFURLPOSIXPathStyle);
  const char* path = CFStringGetCStringPtr(mac_path, CFStringGetSystemEncoding());
  QString bundle_path = QString::fromUtf8(path);
  CFRelease(app_url);
  CFRelease(mac_path);
  return bundle_path;
}

QString GetResourcesPath() {
  QString bundle_path = GetBundlePath();
  return bundle_path + "/Contents/Resources";
}

QString GetApplicationSupportPath() {
  NSAutoreleasePool* pool = [NSAutoreleasePool alloc];
  [pool init];
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory,
      NSUserDomainMask,
      YES);
  QString ret;
  if ([paths count] > 0) {
    NSString* user_path = [paths objectAtIndex:0];
    ret = QString::fromUtf8([user_path UTF8String]);
  } else {
    ret = "~/Library/Application Support";
  }
  [pool drain];
  return ret;
}

QString GetMusicDirectory() {
  NSAutoreleasePool* pool = [NSAutoreleasePool alloc];
  [pool init];
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSMusicDirectory,
      NSUserDomainMask,
      YES);
  QString ret;
  if ([paths count] > 0) {
    NSString* user_path = [paths objectAtIndex:0];
    ret = QString::fromUtf8([user_path UTF8String]);
  } else {
    ret = "~/Music";
  }
  [pool drain];
  return ret;
}

bool MigrateLegacyConfigFiles() {
  bool moved_dir = false;
  QString old_config_dir = QString("%1/.config/%2").arg(
      QDir::homePath(), QCoreApplication::organizationName());
  if (QFile::exists(old_config_dir)) {
    QString new_config_dir = Utilities::GetConfigPath(Utilities::Path_Root);
    // Create ~/Library/Application Support which should already exist anyway.
    QDir::root().mkpath(GetApplicationSupportPath());

    qDebug() << "Move from:" << old_config_dir
             << "to:" << new_config_dir;

    NSFileManager* file_manager = [[NSFileManager alloc] init];
    NSError* error;
    bool ret = [file_manager moveItemAtPath:
        [NSString stringWithUTF8String: old_config_dir.toUtf8().constData()]
        toPath:[NSString stringWithUTF8String: new_config_dir.toUtf8().constData()]
        error: &error];
    if (!ret) {
      qWarning() << [[error localizedDescription] UTF8String];
    }
    moved_dir = true;
  }

  QString old_config_path = QDir::homePath() + "/Library/Preferences/com.davidsansome.Clementine.plist";
  if (QFile::exists(old_config_path)) {
    QSettings settings;
    bool ret = QFile::rename(old_config_path, settings.fileName());
    if (ret) {
      qWarning() << "Migrated old config file: " << old_config_path << "to: " << settings.fileName();
    }
  }

  return moved_dir;
}

}  // namespace mac
