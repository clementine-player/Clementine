/* This file is part of Clementine.

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
      : global_monitor_(nil),
        local_monitor_(nil),
        backend_(backend) {
  }

  bool Register() {
  #ifdef SNOW_LEOPARD
    global_monitor_ = [NSEvent addGlobalMonitorForEventsMatchingMask:NSKeyDownMask
        handler:^(NSEvent* event) {
      HandleKeyEvent(event);
    }];
    local_monitor_ = [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask
        handler:^(NSEvent* event) {
      HandleKeyEvent(event);
      return event;
    }];
    return true;
  #else
    return false;
  #endif
  }

  void Unregister() {
  #ifdef SNOW_LEOPARD
    [NSEvent removeMonitor:global_monitor_];
    [NSEvent removeMonitor:local_monitor_];
  #endif
  }

 private:
  static QKeySequence GetSequence(NSEvent* event) {
    NSString* str = [event charactersIgnoringModifiers];
    NSString* upper = [str uppercaseString];
    const char* chars = [upper UTF8String];
    NSUInteger modifiers = [event modifierFlags];
    int key = 0;
    unsigned char c = chars[0];
    switch (c) {
      case 0x1b: key = Qt::Key_Escape; break;
      case 0x09: key = Qt::Key_Tab; break;
      case 0x0d: key = Qt::Key_Return; break;
      case 0x08: key = Qt::Key_Backspace; break;
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

  void HandleKeyEvent(NSEvent* event) {
    QKeySequence sequence = GetSequence(event);
    backend_->KeyPressed(sequence);
  }

  static int MapFunctionKey(int keycode) {
    switch (keycode) {
      // Function keys
      case NSInsertFunctionKey: return Qt::Key_Insert;
      case NSDeleteFunctionKey: return Qt::Key_Delete;
      case NSPauseFunctionKey: return Qt::Key_Pause;
      case NSPrintFunctionKey: return Qt::Key_Print;
      case NSSysReqFunctionKey: return Qt::Key_SysReq;
      case NSHomeFunctionKey: return Qt::Key_Home;
      case NSEndFunctionKey: return Qt::Key_End;
      case NSLeftArrowFunctionKey: return Qt::Key_Left;
      case NSUpArrowFunctionKey: return Qt::Key_Up;
      case NSRightArrowFunctionKey: return Qt::Key_Right;
      case NSDownArrowFunctionKey: return Qt::Key_Down;
      case NSPageUpFunctionKey: return Qt::Key_PageUp;
      case NSPageDownFunctionKey: return Qt::Key_PageDown;
      case NSScrollLockFunctionKey: return Qt::Key_ScrollLock;
      case NSF1FunctionKey: return Qt::Key_F1;
      case NSF2FunctionKey: return Qt::Key_F2;
      case NSF3FunctionKey: return Qt::Key_F3;
      case NSF4FunctionKey: return Qt::Key_F4;
      case NSF5FunctionKey: return Qt::Key_F5;
      case NSF6FunctionKey: return Qt::Key_F6;
      case NSF7FunctionKey: return Qt::Key_F7;
      case NSF8FunctionKey: return Qt::Key_F8;
      case NSF9FunctionKey: return Qt::Key_F9;
      case NSF10FunctionKey: return Qt::Key_F10;
      case NSF11FunctionKey: return Qt::Key_F11;
      case NSF12FunctionKey: return Qt::Key_F12;
      case NSF13FunctionKey: return Qt::Key_F13;
      case NSF14FunctionKey: return Qt::Key_F14;
      case NSF15FunctionKey: return Qt::Key_F15;
      case NSF16FunctionKey: return Qt::Key_F16;
      case NSF17FunctionKey: return Qt::Key_F17;
      case NSF18FunctionKey: return Qt::Key_F18;
      case NSF19FunctionKey: return Qt::Key_F19;
      case NSF20FunctionKey: return Qt::Key_F20;
      case NSF21FunctionKey: return Qt::Key_F21;
      case NSF22FunctionKey: return Qt::Key_F22;
      case NSF23FunctionKey: return Qt::Key_F23;
      case NSF24FunctionKey: return Qt::Key_F24;
      case NSF25FunctionKey: return Qt::Key_F25;
      case NSF26FunctionKey: return Qt::Key_F26;
      case NSF27FunctionKey: return Qt::Key_F27;
      case NSF28FunctionKey: return Qt::Key_F28;
      case NSF29FunctionKey: return Qt::Key_F29;
      case NSF30FunctionKey: return Qt::Key_F30;
      case NSF31FunctionKey: return Qt::Key_F31;
      case NSF32FunctionKey: return Qt::Key_F32;
      case NSF33FunctionKey: return Qt::Key_F33;
      case NSF34FunctionKey: return Qt::Key_F34;
      case NSF35FunctionKey: return Qt::Key_F35;
      case NSMenuFunctionKey: return Qt::Key_Menu;
      case NSHelpFunctionKey: return Qt::Key_Help;
    }

    return 0;
  }

  id global_monitor_;
  id local_monitor_;
  MacGlobalShortcutBackend* backend_;
};

MacGlobalShortcutBackend::MacGlobalShortcutBackend(GlobalShortcuts* parent)
  : GlobalShortcutBackend(parent),
    p_(new MacGlobalShortcutBackendPrivate(this)) {
}

MacGlobalShortcutBackend::~MacGlobalShortcutBackend() {
}

bool MacGlobalShortcutBackend::DoRegister() {
  // Always enable media keys.
  mac::SetShortcutHandler(this);

  if (AXAPIEnabled()) {
    foreach (const GlobalShortcuts::Shortcut& shortcut, manager_->shortcuts().values()) {
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

void MacGlobalShortcutBackend::KeyPressed(const QKeySequence& sequence) {
  QAction* action = shortcuts_[sequence];
  if (action) {
    action->trigger();
  }
}

bool MacGlobalShortcutBackend::IsAccessibilityEnabled() const {
#ifdef SNOW_LEOPARD
  return AXAPIEnabled();
#else
  return true;  // It's not really enabled but it doesn't matter.
#endif
}

void MacGlobalShortcutBackend::ShowAccessibilityDialog() {
#ifdef SNOW_LEOPARD
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSPreferencePanesDirectory, NSSystemDomainMask, YES);
  if ([paths count] == 1) {
    NSURL* prefpane_url = [NSURL fileURLWithPath:
        [[paths objectAtIndex:0] stringByAppendingPathComponent:@"UniversalAccessPref.prefPane"]];
    [[NSWorkspace sharedWorkspace] openURL:prefpane_url];
  }
#endif
}
