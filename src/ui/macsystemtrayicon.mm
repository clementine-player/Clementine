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

#include "macsystemtrayicon.h"

#include <QAction>
#include <QApplication>
#include <QIcon>

#include <QtDebug>

#include <AppKit/NSMenu.h>
#include <AppKit/NSMenuItem.h>

@interface Target :NSObject {
  QAction* action_;
}
- (id) initWithQAction: (QAction*)action;
- (void) clicked;
@end

@implementation Target  // <NSMenuValidation>
- (id) init {
  return [super init];
}

- (id) initWithQAction: (QAction*)action {
  action_ = action;
  return self;
}

- (BOOL) validateMenuItem: (NSMenuItem*)menuItem {
  // This is called when the menu is shown.
  return action_->isEnabled();
}

- (void) clicked {
  action_->trigger();
}
@end

class MacSystemTrayIconPrivate : boost::noncopyable {
 public:
  MacSystemTrayIconPrivate() {
    dock_menu_ = [[NSMenu alloc] initWithTitle:@"DockMenu"];

    // Don't look now.
    // This must be called after our custom NSApplicationDelegate has been set.
    [[NSApp delegate] setDockMenu:dock_menu_];
  }

  void AddMenuItem(QAction* action) {
    // Strip accelarators from name.
    QString text = action->text().remove("&");
    NSString* title = [[NSString alloc] initWithUTF8String: text.toUtf8().constData()];
    // Create an object that can receive user clicks and pass them on to the QAction.
    Target* target = [[Target alloc] initWithQAction:action];
    NSMenuItem* item = [[[NSMenuItem alloc]
        initWithTitle:title
        action:@selector(clicked)
        keyEquivalent:@""] autorelease];
    [item setEnabled:action->isEnabled()];
    [item setTarget:target];
    [dock_menu_ addItem:item];
    actions_[action] = item;
  }

  void ActionChanged(QAction* action) {
    NSMenuItem* item = actions_[action];
    [[item title] release];
    NSString* title = [[NSString alloc] initWithUTF8String: action->text().toUtf8().constData()];
    [item setTitle:title];
  }

  void AddSeparator() {
    NSMenuItem* separator = [NSMenuItem separatorItem];
    [dock_menu_ addItem:separator];
  }

 private:
  QMap<QAction*, NSMenuItem*> actions_;

  NSMenu* dock_menu_;
};

MacSystemTrayIcon::MacSystemTrayIcon(QObject* parent)
  : SystemTrayIcon(parent),
    orange_icon_(QPixmap(":icon_large.png").scaled(
        128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
    grey_icon_(QPixmap(":icon_large_grey.png").scaled(
        128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation)) {
  QApplication::setWindowIcon(orange_icon_);
}

MacSystemTrayIcon::~MacSystemTrayIcon() {
}

void MacSystemTrayIcon::SetupMenu(
    QAction* previous, QAction* play, QAction* stop, QAction* stop_after,
    QAction* next, QAction* love, QAction* ban, QAction* quit) {
  p_.reset(new MacSystemTrayIconPrivate());
  SetupMenuItem(previous);
  SetupMenuItem(play);
  SetupMenuItem(stop);
  SetupMenuItem(stop_after);
  SetupMenuItem(next);
  p_->AddSeparator();
  SetupMenuItem(love);
  SetupMenuItem(ban);
  Q_UNUSED(quit);  // Mac already has a Quit item.
}

void MacSystemTrayIcon::SetupMenuItem(QAction* action) {
  p_->AddMenuItem(action);
  connect(action, SIGNAL(changed()), SLOT(ActionChanged()));
}

void MacSystemTrayIcon::UpdateIcon() {
  QApplication::setWindowIcon(CreateIcon(orange_icon_, grey_icon_));
}

void MacSystemTrayIcon::ActionChanged() {
  QAction* action = qobject_cast<QAction*>(sender());
  p_->ActionChanged(action);
}
