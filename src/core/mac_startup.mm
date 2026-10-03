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
#import <AppKit/NSView.h>
#import <AppKit/NSViewController.h>
#import <AppKit/NSWindow.h>

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

#import <Security/Security.h>


#include "config.h"
#include "core/logging.h"
#include "core/scoped_cftyperef.h"
#include "core/scoped_nsautorelease_pool.h"
#include "globalshortcuts.h"
#include "mac_delegate.h"
#include "mac_startup.h"
#include "mac_utilities.h"
#include "macglobalshortcutbackend.h"
#include "utilities.h"

#ifdef HAVE_SPARKLE
#import <Sparkle/SPUStandardUpdaterController.h>
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QSet>
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

  return self;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)app hasVisibleWindows:(BOOL)flag {
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

- (BOOL)application:(NSApplication*)app openFile:(NSString*)filename {
  qLog(Debug) << "Wants to open:" << [filename UTF8String];

  if (application_handler_->LoadUrl(QString::fromUtf8([filename UTF8String]))) {
    return YES;
  }

  return NO;
}

- (void)application:(NSApplication*)app openFiles:(NSArray*)filenames {
  qLog(Debug) << "Wants to open:" << filenames;
  [filenames enumerateObjectsUsingBlock:^(id object, NSUInteger idx, BOOL* stop) {
    [self application:app openFile:(NSString*)object];
  }];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
#ifdef HAVE_BREAKPAD
  BreakpadRelease(breakpad_);
#endif
  return NSTerminateNow;
}

- (void)userNotificationCenter:(UNUserNotificationCenter*)center
       willPresentNotification:(UNNotification*)notification
         withCompletionHandler:(void (^)(UNNotificationPresentationOptions))completionHandler {
  // Always show notifications, even if Clementine is in the foreground.
  completionHandler(UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionList);
}

- (void)userNotificationCenter:(UNUserNotificationCenter*)center
    didReceiveNotificationResponse:(UNNotificationResponse*)response
             withCompletionHandler:(void (^)(void))completionHandler {
  // Clicking a notification brings Clementine to the front.
  if (application_handler_) {
    application_handler_->Activate();
  }
  completionHandler();
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

  [UNUserNotificationCenter currentNotificationCenter].delegate = delegate_;
}

@end

namespace mac {

#ifdef HAVE_SPARKLE
// Lives as long as the app: it checks for updates on Sparkle's schedule.
static SPUStandardUpdaterController* updater_controller = nil;
#endif

void MacMain() {
  ScopedNSAutoreleasePool pool;
  // Creates and sets the magic global variable so QApplication will find it.
  [MacApplication sharedApplication];
#ifdef HAVE_SPARKLE
  updater_controller = [[SPUStandardUpdaterController alloc] initWithStartingUpdater:YES
                                                                     updaterDelegate:nil
                                                                  userDriverDelegate:nil];
#endif
}

void SetShortcutHandler(MacGlobalShortcutBackend* handler) { [NSApp SetShortcutHandler:handler]; }

void SetApplicationHandler(PlatformInterface* handler) { [NSApp SetApplicationHandler:handler]; }

void CheckForUpdates() {
#ifdef HAVE_SPARKLE
  [updater_controller checkForUpdates:nil];
#endif
}

QString GetBundlePath() {
  ScopedCFTypeRef<CFURLRef> app_url(CFBundleCopyBundleURL(CFBundleGetMainBundle()));
  ScopedCFTypeRef<CFStringRef> mac_path(
      CFURLCopyFileSystemPath(app_url.get(), kCFURLPOSIXPathStyle));
  const char* path = CFStringGetCStringPtr(mac_path.get(), CFStringGetSystemEncoding());
  QString bundle_path = QString::fromUtf8(path);
  return bundle_path;
}

QString GetResourcesPath() {
  QString bundle_path = GetBundlePath();
  return bundle_path + "/Contents/Resources";
}

QString GetApplicationSupportPath() {
  ScopedNSAutoreleasePool pool;
  NSArray* paths =
      NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
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
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSMusicDirectory, NSUserDomainMask, YES);
  QString ret;
  if ([paths count] > 0) {
    NSString* user_path = [paths objectAtIndex:0];
    ret = QString::fromUtf8([user_path UTF8String]);
  } else {
    ret = "~/Music";
  }
  return ret;
}

namespace {

QByteArray CertificateData(SecCertificateRef certificate) {
  ScopedCFTypeRef<CFDataRef> data(SecCertificateCopyData(certificate));
  return QByteArray(reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
                    CFDataGetLength(data));
}

bool IsTlsPolicy(SecPolicyRef policy) {
  ScopedCFTypeRef<CFDictionaryRef> properties(SecPolicyCopyProperties(policy));
  CFTypeRef oid = properties ? CFDictionaryGetValue(properties, kSecPolicyOid) : nullptr;
  return oid && CFEqual(oid, kSecPolicyAppleSSL);
}

enum class TlsTrust { Unspecified, Trusted, Distrusted };

// What a certificate's trust settings in one domain say about trusting it
// as a root for TLS connections. See SecTrustSettingsCopyTrustSettings.
TlsTrust TlsTrustFrom(CFArrayRef settings) {
  // No constraints at all: trusted as a root for everything.
  if (CFArrayGetCount(settings) == 0) return TlsTrust::Trusted;

  for (CFIndex i = 0; i < CFArrayGetCount(settings); ++i) {
    CFDictionaryRef constraint =
        static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(settings, i));
    // Settings for one app only, or for something other than TLS.
    if (CFDictionaryContainsKey(constraint, kSecTrustSettingsApplication)) continue;
    CFTypeRef policy = CFDictionaryGetValue(constraint, kSecTrustSettingsPolicy);
    if (policy && !IsTlsPolicy(static_cast<SecPolicyRef>(const_cast<void*>(policy)))) {
      continue;
    }

    SInt32 result = kSecTrustSettingsResultTrustRoot;
    CFNumberRef number =
        static_cast<CFNumberRef>(CFDictionaryGetValue(constraint, kSecTrustSettingsResult));
    if (number) CFNumberGetValue(number, kCFNumberSInt32Type, &result);
    switch (result) {
      case kSecTrustSettingsResultTrustRoot:
      case kSecTrustSettingsResultTrustAsRoot:
        return TlsTrust::Trusted;
      case kSecTrustSettingsResultDeny:
        return TlsTrust::Distrusted;
      default:
        break;
    }
  }
  return TlsTrust::Unspecified;
}

}  // namespace

QList<QByteArray> GetTrustedRootCertificates() {
  QSet<QByteArray> trusted;

  CFArrayRef anchors = nullptr;
  OSStatus status = SecTrustCopyAnchorCertificates(&anchors);
  ScopedCFTypeRef<CFArrayRef> scoped_anchors(anchors);
  if (status == errSecSuccess) {
    for (CFIndex i = 0; i < CFArrayGetCount(anchors); ++i) {
      trusted.insert(CertificateData(
          static_cast<SecCertificateRef>(const_cast<void*>(CFArrayGetValueAtIndex(anchors, i)))));
    }
  } else {
    qLog(Error) << "Couldn't get the system's root certificates:" << status;
  }

  // The user's settings win over an admin's, which win over the system's.
  for (SecTrustSettingsDomain domain : {kSecTrustSettingsDomainAdmin, kSecTrustSettingsDomainUser}) {
    CFArrayRef certificates = nullptr;
    // errSecNoTrustSettings when the domain has none.
    if (SecTrustSettingsCopyCertificates(domain, &certificates) != errSecSuccess) continue;
    ScopedCFTypeRef<CFArrayRef> scoped_certificates(certificates);

    for (CFIndex i = 0; i < CFArrayGetCount(certificates); ++i) {
      SecCertificateRef certificate = static_cast<SecCertificateRef>(
          const_cast<void*>(CFArrayGetValueAtIndex(certificates, i)));
      CFArrayRef settings = nullptr;
      if (SecTrustSettingsCopyTrustSettings(certificate, domain, &settings) != errSecSuccess) {
        continue;
      }
      ScopedCFTypeRef<CFArrayRef> scoped_settings(settings);
      switch (TlsTrustFrom(settings)) {
        case TlsTrust::Trusted:
          trusted.insert(CertificateData(certificate));
          break;
        case TlsTrust::Distrusted:
          trusted.remove(CertificateData(certificate));
          break;
        case TlsTrust::Unspecified:
          break;
      }
    }
  }

  return trusted.values();
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

int GetWindowNumber(QWidget* widget) {
  NSView* view = reinterpret_cast<NSView*>(widget->winId());
  return [[view window] windowNumber];
}

}  // namespace mac
