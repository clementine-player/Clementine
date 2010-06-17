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

#include "globalshortcuts.h"
#include "mac_startup.h"

#include <boost/noncopyable.hpp>

#include <QAction>
#include <QList>
#include <QtDebug>

#include <AppKit/NSEvent.h>
#include <Foundation/NSString.h>
#include <IOKit/hidsystem/ev_keymap.h>

class MacGlobalShortcutBackendPrivate : boost::noncopyable {
 public:
  explicit MacGlobalShortcutBackendPrivate(MacGlobalShortcutBackend* backend)
      : monitor_(nil),
        backend_(backend) {
  }

  bool Register() {
  #ifdef NS_BLOCKS_AVAILABLE
    monitor_ = [NSEvent addGlobalMonitorForEventsMatchingMask:NSKeyDownMask
        handler:^(NSEvent* event) {
      qDebug() << __PRETTY_FUNCTION__;
      HandleKeyEvent(event);
    }];
    return true;
  #else
    return false;
  #endif
  }

  void Unregister() {
    [NSEvent removeMonitor:monitor_];
  }

 private:
  static QKeySequence GetSequence(NSEvent* event) {
    NSString* str = [event charactersIgnoringModifiers];
    NSString* lower = [str lowercaseString];
    const char* chars = [lower UTF8String];
    NSUInteger modifiers = [event modifierFlags];
    int key = Qt::Key_A + chars[0] - 'a';  // >.>
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

  id monitor_;
  MacGlobalShortcutBackend* backend_;
};

MacGlobalShortcutBackend::MacGlobalShortcutBackend(GlobalShortcuts* parent)
  : GlobalShortcutBackend(parent),
    p_(new MacGlobalShortcutBackendPrivate(this)) {
}

MacGlobalShortcutBackend::~MacGlobalShortcutBackend() {
  delete p_;
}

bool MacGlobalShortcutBackend::DoRegister() {
  mac::SetShortcutHandler(this);
  foreach (const GlobalShortcuts::Shortcut& shortcut, manager_->shortcuts().values()) {
    shortcuts_[shortcut.action->shortcut()] = shortcut.action;
  }
  return p_->Register();
}

void MacGlobalShortcutBackend::DoUnregister() {
  p_->Unregister();
}

void MacGlobalShortcutBackend::MacMediaKeyPressed(int key) {
  switch (key) {
    case NX_KEYTYPE_PLAY:
      manager_->shortcuts()["play_pause"].action->trigger();
      break;
    case NX_KEYTYPE_FAST:
      manager_->shortcuts()["next_track"].action->trigger();
      break;
    case NX_KEYTYPE_REWIND:
      manager_->shortcuts()["prev_track"].action->trigger();
      break;
  }
}

void MacGlobalShortcutBackend::KeyPressed(const QKeySequence& sequence) {
  QAction* action = shortcuts_[sequence];
  if (action) {
    action->trigger();
  }
}
