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

#include "macsystemtrayicon.h"

#include "core/mac_delegate.h"
#include "core/song.h"

#include <QAction>
#include <QApplication>
#include <QIcon>

#include <QtDebug>

#include <AppKit/AppKit.h>
#include <QGuiApplication>
#include <QImage>
#include <QStyleHints>

@interface Target : NSObject {
  QAction* action_;
}
- (id)initWithQAction:(QAction*)action;
- (void)clicked;
@end

@implementation Target  // <NSMenuValidation>
- (id)init {
  return [super init];
}

- (id)initWithQAction:(QAction*)action {
  action_ = action;
  return self;
}

- (BOOL)validateMenuItem:(NSMenuItem*)menuItem {
  // This is called when the menu is shown.
  return action_->isEnabled();
}

- (void)clicked {
  action_->trigger();
}
@end

class MacSystemTrayIconPrivate {
 public:
  MacSystemTrayIconPrivate() {
    dock_menu_ = [[NSMenu alloc] initWithTitle:@"DockMenu"];

    QString title = QT_TR_NOOP("Now Playing");
    NSString* t = [[NSString alloc] initWithUTF8String:title.toUtf8().constData()];
    now_playing_ = [[NSMenuItem alloc] initWithTitle:t action:nullptr keyEquivalent:@""];

    now_playing_artist_ = [[NSMenuItem alloc] initWithTitle:@"Nothing to see here"
                                                     action:nullptr
                                              keyEquivalent:@""];

    now_playing_title_ = [[NSMenuItem alloc] initWithTitle:@"Nothing to see here"
                                                    action:nullptr
                                             keyEquivalent:@""];

    [dock_menu_ insertItem:now_playing_title_ atIndex:0];
    [dock_menu_ insertItem:now_playing_artist_ atIndex:0];
    [dock_menu_ insertItem:now_playing_ atIndex:0];

    // Don't look now.
    // This must be called after our custom NSApplicationDelegate has been set.
    [(AppDelegate*)([NSApp delegate]) setDockMenu:dock_menu_];

    ClearNowPlaying();
  }

  void AddMenuItem(QAction* action) {
    // Strip accelarators from name.
    QString text = action->text().remove("&");
    NSString* title = [[NSString alloc] initWithUTF8String:text.toUtf8().constData()];
    // Create an object that can receive user clicks and pass them on to the
    // QAction.
    Target* target = [[Target alloc] initWithQAction:action];
    NSMenuItem* item = [[[NSMenuItem alloc] initWithTitle:title
                                                   action:@selector(clicked)
                                            keyEquivalent:@""] autorelease];
    [item setEnabled:action->isEnabled()];
    [item setTarget:target];
    [dock_menu_ addItem:item];
    actions_[action] = item;
  }

  void ActionChanged(QAction* action) {
    NSMenuItem* item = actions_[action];
    NSString* title = [[NSString alloc] initWithUTF8String:action->text().toUtf8().constData()];
    [item setTitle:title];
  }

  void AddSeparator() {
    NSMenuItem* separator = [NSMenuItem separatorItem];
    [dock_menu_ addItem:separator];
  }

  void ShowNowPlaying(const QString& artist, const QString& title) {
    ClearNowPlaying();  // Makes sure the order is consistent.
    [now_playing_artist_
        setTitle:[[NSString alloc] initWithUTF8String:artist.toUtf8().constData()]];
    [now_playing_title_ setTitle:[[NSString alloc] initWithUTF8String:title.toUtf8().constData()]];
    title.isEmpty() ? HideItem(now_playing_title_) : ShowItem(now_playing_title_);
    artist.isEmpty() ? HideItem(now_playing_artist_) : ShowItem(now_playing_artist_);
    artist.isEmpty() && title.isEmpty() ? HideItem(now_playing_) : ShowItem(now_playing_);
  }

  void ClearNowPlaying() {
    // Hiding doesn't seem to work in the dock menu.
    HideItem(now_playing_);
    HideItem(now_playing_artist_);
    HideItem(now_playing_title_);
  }

 private:
  void HideItem(NSMenuItem* item) {
    if ([dock_menu_ indexOfItem:item] != -1) {
      [dock_menu_ removeItem:item];
    }
  }

  void ShowItem(NSMenuItem* item, int index = 0) {
    if ([dock_menu_ indexOfItem:item] == -1) {
      [dock_menu_ insertItem:item atIndex:index];
    }
  }

  QMap<QAction*, NSMenuItem*> actions_;

  NSMenu* dock_menu_;
  NSMenuItem* now_playing_;
  NSMenuItem* now_playing_artist_;
  NSMenuItem* now_playing_title_;

  Q_DISABLE_COPY(MacSystemTrayIconPrivate);
};

namespace {

// The Dock shows icons at up to 128 points, which is 256 pixels on Retina
// screens.
const int kDockIconSize = 256;

// Puts a circular play or pause badge in the icon's bottom right, dark on a
// light Dock and light on a dark one, so it stands out either way.
QPixmap AddPlaybackBadge(const QPixmap& icon, bool playing, bool dark) {
  QImage image = icon.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
  const CGFloat size = image.width();

  CGColorSpaceRef color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  CGContextRef context = CGBitmapContextCreate(
      image.bits(), image.width(), image.height(), 8, image.bytesPerLine(), color_space,
      kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
  CGColorSpaceRelease(color_space);

  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:context
                                                                               flipped:NO]];

  NSColor* fill =
      dark ? [NSColor colorWithWhite:0.98 alpha:1.0] : [NSColor colorWithWhite:0.13 alpha:0.94];
  NSColor* glyph_color = dark ? [NSColor colorWithWhite:0.13 alpha:1.0] : [NSColor whiteColor];

  const CGFloat diameter = size * 0.42;
  const CGFloat inset = size * 0.03;
  const NSRect badge = NSMakeRect(size - diameter - inset, inset, diameter, diameter);

  [NSGraphicsContext saveGraphicsState];
  NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
  shadow.shadowBlurRadius = size * 0.025;
  shadow.shadowOffset = NSMakeSize(0, -size * 0.008);
  shadow.shadowColor = [NSColor colorWithWhite:0 alpha:0.35];
  [shadow set];
  [fill setFill];
  [[NSBezierPath bezierPathWithOvalInRect:badge] fill];
  [NSGraphicsContext restoreGraphicsState];

  NSImageSymbolConfiguration* config = [[NSImageSymbolConfiguration
      configurationWithPointSize:diameter * 0.36
                          weight:NSFontWeightBold]
      configurationByApplyingConfiguration:[NSImageSymbolConfiguration
                                               configurationWithPaletteColors:@[ glyph_color ]]];
  NSImage* glyph = [[NSImage imageWithSystemSymbolName:playing ? @"play.fill" : @"pause.fill"
                              accessibilityDescription:nil] imageWithSymbolConfiguration:config];
  // The play triangle's weight is left of its centre, so nudge it right.
  const CGFloat nudge = playing ? diameter * 0.025 : 0;
  [glyph drawInRect:NSMakeRect(NSMidX(badge) - glyph.size.width / 2 + nudge,
                               NSMidY(badge) - glyph.size.height / 2, glyph.size.width,
                               glyph.size.height)];

  [NSGraphicsContext restoreGraphicsState];
  CGContextRelease(context);
  return QPixmap::fromImage(image);
}

}  // namespace

MacSystemTrayIcon::MacSystemTrayIcon(QObject* parent)
    : SystemTrayIcon(parent),
      orange_icon_(
          QPixmap(":icon_large.png")
              .scaled(kDockIconSize, kDockIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
      grey_icon_(QPixmap(":icon_large_grey.png")
                     .scaled(kDockIconSize, kDockIconSize, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation)) {
  // The badge contrasts with the Dock, which follows the system's light or
  // dark appearance. So does Qt's colour scheme, unless Clementine's own
  // Appearance setting forces one, when the badge follows that instead.
  connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
          [this] { UpdateIcon(); });
  UpdateIcon();
}

MacSystemTrayIcon::~MacSystemTrayIcon() {}

void MacSystemTrayIcon::SetupMenu(QAction* previous, QAction* play, QAction* stop,
                                  QAction* stop_after, QAction* next, QAction* mute, QAction* love,
                                  QAction* quit) {
  p_.reset(new MacSystemTrayIconPrivate());
  SetupMenuItem(previous);
  SetupMenuItem(play);
  SetupMenuItem(stop);
  SetupMenuItem(stop_after);
  SetupMenuItem(next);
  p_->AddSeparator();
  SetupMenuItem(mute);
  p_->AddSeparator();
  SetupMenuItem(love);
  Q_UNUSED(quit);  // Mac already has a Quit item.
}

void MacSystemTrayIcon::SetupMenuItem(QAction* action) {
  p_->AddMenuItem(action);
  connect(action, SIGNAL(changed()), SLOT(ActionChanged()));
}

void MacSystemTrayIcon::UpdateIcon() {
  QPixmap icon = CreateProgressIcon(orange_icon_, grey_icon_);
  if (playback_state() != PlaybackState::Stopped) {
    const bool dark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    icon = AddPlaybackBadge(icon, playback_state() == PlaybackState::Playing, dark);
  }
  QApplication::setWindowIcon(icon);
}

void MacSystemTrayIcon::ActionChanged() {
  QAction* action = qobject_cast<QAction*>(sender());
  p_->ActionChanged(action);
}

void MacSystemTrayIcon::ClearNowPlaying() { p_->ClearNowPlaying(); }

void MacSystemTrayIcon::SetNowPlaying(const Song& song, const QString& image_path) {
  p_->ShowNowPlaying(song.artist(), song.PrettyTitle());
}
