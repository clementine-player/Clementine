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
  #ifdef NS_BLOCKS_AVAILABLE
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
    [NSEvent removeMonitor:global_monitor_];
    [NSEvent removeMonitor:local_monitor_];
  }

  // See UIElementInspector example.
  bool CheckAccessibilityEnabled() {
    if (AXAPIEnabled()) {
      return true;
    }

    QMessageBox box(
        QMessageBox::Question,
        QObject::tr("Accessibility API required for global shortcuts"),
        QObject::tr("Would you like to launch System Preferences so that you can turn on"
                    " \"Enable access for assistive devices\"?\n"
                    "This is required to use global shortcuts in Clementine."));
    QPushButton* default_button =
        box.addButton(QObject::tr("Open System Preferences"), QMessageBox::AcceptRole);
    QPushButton* continue_button =
        box.addButton(QObject::tr("Continue anyway"), QMessageBox::RejectRole);
    box.setDefaultButton(default_button);

    box.exec();
    QPushButton* clicked_button = static_cast<QPushButton*>(box.clickedButton());
    if (clicked_button == default_button) {
      NSArray* paths = NSSearchPathForDirectoriesInDomains(
          NSPreferencePanesDirectory, NSSystemDomainMask, YES);
      if ([paths count] == 1) {
        NSURL* prefpane_url = [NSURL fileURLWithPath:
            [[paths objectAtIndex:0] stringByAppendingPathComponent:@"UniversalAccessPref.prefPane"]];
        [[NSWorkspace sharedWorkspace] openURL:prefpane_url];
      }
      // We assume the user actually clicks the button in the preference pane here...
      return true;
    } else if (clicked_button == continue_button) {
      return false;
    }

    return false;
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

  id global_monitor_;
  id local_monitor_;
  MacGlobalShortcutBackend* backend_;
};

MacGlobalShortcutBackend::MacGlobalShortcutBackend(GlobalShortcuts* parent)
  : GlobalShortcutBackend(parent),
    accessibility_status_(NOT_CHECKED),
    p_(new MacGlobalShortcutBackendPrivate(this)) {
}

MacGlobalShortcutBackend::~MacGlobalShortcutBackend() {
}

bool MacGlobalShortcutBackend::DoRegister() {
  // Always enable media keys.
  mac::SetShortcutHandler(this);

  // Check whether universal access is enabled so that global shortcuts will work.
  // This may pop up a modal dialog so only ask once per session.
  if (accessibility_status_ == NOT_CHECKED) {
    accessibility_status_ = CheckAccessibilityEnabled() ? ENABLED : DISABLED;
  }

  if (accessibility_status_ == ENABLED && AXAPIEnabled()) {
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

bool MacGlobalShortcutBackend::CheckAccessibilityEnabled() {
  return p_->CheckAccessibilityEnabled();
}
