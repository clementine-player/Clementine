/* This file is part of Clementine.
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "appearance.h"

#include <QApplication>
#include <QSettings>
#include <QStyleHints>
#include <QWidget>

// Needed for HAVE_DBUS: without it the portal support below silently compiles
// out and "Follow system" quietly falls back to light.
#include "clementine-config.h"
#include "core/logging.h"

#ifdef HAVE_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#endif

const char* Appearance::kSettingsGroup = "Appearance";
const char* Appearance::kUseCustomColorSet = "use-custom-set";
const char* Appearance::kForegroundColor = "foreground-color";
const char* Appearance::kBackgroundColor = "background-color";
const char* Appearance::kThemeMode = "theme-mode";

const QPalette Appearance::kDefaultPalette = QPalette();

Appearance::Appearance(QObject* parent) : QObject(parent) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QPalette p = QApplication::palette();
  background_color_ =
      s.value(kBackgroundColor, p.color(QPalette::WindowText)).value<QColor>();
  foreground_color_ =
      s.value(kForegroundColor, p.color(QPalette::Window)).value<QColor>();
  theme_mode_ =
      static_cast<ThemeMode>(s.value(kThemeMode, ThemeMode_System).toInt());

  RefreshDesktopScheme();

#ifdef HAVE_DBUS
  // Follow the desktop live. SettingChanged carries the new value, but the
  // slot takes no arguments and re-reads instead - see the header.
  QDBusConnection::sessionBus().connect(
      "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
      "org.freedesktop.portal.Settings", "SettingChanged", this,
      SLOT(DesktopSettingChanged()));
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  // Follow the desktop live, so switching the system to dark at sunset moves
  // the app with it rather than waiting for a restart. Only meaningful while
  // the user is on System; the lambda re-checks rather than disconnecting so
  // that switching back to System picks up tracking again.
  connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
          [this](Qt::ColorScheme) {
            if (theme_mode_ == ThemeMode_System) LoadUserTheme();
          });
#endif
}

namespace {
#ifdef HAVE_DBUS
// Values of org.freedesktop.appearance/color-scheme, per the xdg-desktop-portal
// spec. 0 means the user hasn't expressed a preference either way.
const uint kPortalNoPreference = 0;
const uint kPortalPreferDark = 1;
#endif
}  // namespace

void Appearance::RefreshDesktopScheme() {
  desktop_scheme_ = ThemeMode_System;  // ie. we don't know

#ifdef HAVE_DBUS
  QDBusInterface portal(
      "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
      "org.freedesktop.portal.Settings", QDBusConnection::sessionBus());
  if (!portal.isValid()) return;

  QDBusReply<QDBusVariant> reply =
      portal.call("Read", "org.freedesktop.appearance", "color-scheme");
  if (!reply.isValid()) return;

  // The portal hands back a variant wrapping a variant wrapping the uint.
  QVariant value = reply.value().variant();
  if (value.canConvert<QDBusVariant>()) {
    value = value.value<QDBusVariant>().variant();
  }

  bool ok = false;
  const uint scheme = value.toUInt(&ok);
  if (!ok || scheme == kPortalNoPreference) return;

  desktop_scheme_ =
      scheme == kPortalPreferDark ? ThemeMode_Dark : ThemeMode_Light;
  qLog(Debug) << "Desktop colour scheme preference:"
              << (desktop_scheme_ == ThemeMode_Dark ? "dark" : "light");
#endif  // HAVE_DBUS
}

void Appearance::DesktopSettingChanged() {
  const ThemeMode previous = desktop_scheme_;
  RefreshDesktopScheme();
  // The portal signals every setting, not just ours, so only rebuild the UI
  // when the answer actually moved and we're the ones following it.
  if (desktop_scheme_ != previous && theme_mode_ == ThemeMode_System) {
    ApplyTheme();
  }
}

Appearance::ThemeMode Appearance::EffectiveTheme() const {
  if (theme_mode_ != ThemeMode_System) return theme_mode_;

  // The desktop portal is asked first and wins. Qt's own colorScheme() only
  // knows the answer when a platform theme plugin is loaded to supply it: on a
  // plain GNOME Wayland session with no QT_QPA_PLATFORMTHEME set it reports
  // Light however the desktop is actually configured, so it cannot be the only
  // source or "Follow system" silently never goes dark on Linux.
  if (desktop_scheme_ != ThemeMode_System) return desktop_scheme_;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
    return ThemeMode_Dark;
  }
#endif
  // Nothing to go on - Qt older than 6.5 can't be asked at all, and distro
  // builds still on Qt 6.2 (Ubuntu jammy) land here.
  return ThemeMode_Light;
}

bool Appearance::IsDarkPalette(const QPalette& palette) {
  // Comparing the two lightnesses rather than testing the background against
  // a fixed threshold keeps this working for custom colour sets, where the
  // user may have picked something that isn't near either extreme.
  return palette.color(QPalette::WindowText).lightness() >
         palette.color(QPalette::Window).lightness();
}

QPalette Appearance::DarkPalette() {
  QPalette p;

  const QColor window(0x35, 0x35, 0x3a);
  const QColor base(0x25, 0x25, 0x2a);
  const QColor text(0xe6, 0xe6, 0xe8);
  const QColor highlight(0x3d, 0x7e, 0xbf);
  const QColor disabled(0x7f, 0x7f, 0x84);

  p.setColor(QPalette::Window, window);
  p.setColor(QPalette::WindowText, text);
  p.setColor(QPalette::Base, base);
  p.setColor(QPalette::AlternateBase, window);
  p.setColor(QPalette::Text, text);
  p.setColor(QPalette::Button, window);
  p.setColor(QPalette::ButtonText, text);
  p.setColor(QPalette::BrightText, Qt::red);
  p.setColor(QPalette::ToolTipBase, base);
  p.setColor(QPalette::ToolTipText, text);
  p.setColor(QPalette::Highlight, highlight);
  p.setColor(QPalette::HighlightedText, Qt::white);
  p.setColor(QPalette::Link, QColor(0x5c, 0x9e, 0xdf));
  p.setColor(QPalette::LinkVisited, QColor(0x9c, 0x7c, 0xd0));

  // The 3D roles. Qt derives these from Button when they aren't set, which
  // for a dark button gives washed out frames - mainwindow.css leans on Mid
  // and Shadow for its borders, so set them explicitly.
  p.setColor(QPalette::Light, window.lighter(140));
  p.setColor(QPalette::Midlight, window.lighter(120));
  p.setColor(QPalette::Mid, window.darker(120));
  p.setColor(QPalette::Dark, window.darker(150));
  p.setColor(QPalette::Shadow, QColor(0x10, 0x10, 0x12));

  p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
  p.setColor(QPalette::Disabled, QPalette::Text, disabled);
  p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
  p.setColor(QPalette::Disabled, QPalette::Highlight, window.lighter(120));
  p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
  p.setColor(QPalette::Disabled, QPalette::Link, disabled);
  p.setColor(QPalette::Disabled, QPalette::LinkVisited, disabled);

  // Qt does not derive PlaceholderText from Text: a role left unset keeps the
  // default black, which is invisible against a dark base. This is what draws
  // the "Search for anything" hint in the global search box, so it has to be
  // set explicitly rather than relying on Text.
  QColor placeholder = text;
  placeholder.setAlpha(128);
  p.setColor(QPalette::PlaceholderText, placeholder);

  return p;
}

QPalette Appearance::LightPalette() {
  QPalette p;

  const QColor window(0xef, 0xef, 0xf0);
  const QColor base(0xff, 0xff, 0xff);
  const QColor text(0x1a, 0x1a, 0x1c);
  const QColor highlight(0x30, 0x8c, 0xc6);
  const QColor disabled(0xa0, 0xa0, 0xa4);

  p.setColor(QPalette::Window, window);
  p.setColor(QPalette::WindowText, text);
  p.setColor(QPalette::Base, base);
  p.setColor(QPalette::AlternateBase, window);
  p.setColor(QPalette::Text, text);
  p.setColor(QPalette::Button, window);
  p.setColor(QPalette::ButtonText, text);
  p.setColor(QPalette::BrightText, Qt::red);
  p.setColor(QPalette::ToolTipBase, base);
  p.setColor(QPalette::ToolTipText, text);
  p.setColor(QPalette::Highlight, highlight);
  p.setColor(QPalette::HighlightedText, Qt::white);
  p.setColor(QPalette::Link, QColor(0x1f, 0x6f, 0xb0));
  p.setColor(QPalette::LinkVisited, QColor(0x76, 0x53, 0xb0));

  p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
  p.setColor(QPalette::Disabled, QPalette::Text, disabled);
  p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
  p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);

  // Qt does not derive PlaceholderText from Text: a role left unset keeps the
  // default black, which is invisible against a dark base. This is what draws
  // the "Search for anything" hint in the global search box, so it has to be
  // set explicitly rather than relying on Text.
  QColor placeholder = text;
  placeholder.setAlpha(128);
  p.setColor(QPalette::PlaceholderText, placeholder);

  return p;
}

QPalette Appearance::BasePalette() const {
  if (EffectiveTheme() == ThemeMode_Dark) return DarkPalette();

  // The system palette is the right light base nearly always, and preserving
  // it keeps the app looking native. The exception is a desktop that is
  // itself dark: there it isn't a light palette at all, so someone who has
  // explicitly asked for Light would otherwise still get a dark window.
  if (IsDarkPalette(kDefaultPalette)) return LightPalette();
  return kDefaultPalette;
}

void Appearance::ApplyTheme() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  // Tell the platform as well as ourselves, so the things a palette can't
  // reach follow too - notably the window title bar on Windows and macOS.
  // This is deliberately not the whole mechanism: it does nothing at all on
  // some platform themes (it's a no-op under the offscreen and plain-Fusion
  // setups this was tested on), so the palette below is what actually
  // guarantees the app is dark. Set it first so ours wins where both apply.
  switch (EffectiveTheme()) {
    case ThemeMode_Dark:
      QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
      break;
    case ThemeMode_Light:
      QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
      break;
    default:
      QGuiApplication::styleHints()->unsetColorScheme();
      break;
  }
#endif

  QApplication::setPalette(BasePalette());

  RepolishStyledWidgets();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.value(kUseCustomColorSet).toBool()) {
    // Layered on top of the base rather than instead of it, so the roles the
    // user hasn't chosen still come from the right theme.
    ChangeForegroundColor(foreground_color_);
    ChangeBackgroundColor(background_color_);
  }
}

void Appearance::RepolishStyledWidgets() {
  // Qt resolves a stylesheet-styled widget's palette when it polishes the
  // widget, and doesn't reliably redo that when the application palette
  // changes underneath it - widgets living outside the main window's hierarchy
  // (the library and icecast filter boxes) end up rendering a theme behind,
  // most visibly as black text on a dark background. Clearing and re-setting
  // the stylesheet forces the resolve to run again against the new palette.
  for (QWidget* widget : QApplication::allWidgets()) {
    const QString stylesheet = widget->styleSheet();
    if (stylesheet.isEmpty()) continue;
    widget->setStyleSheet(QString());
    widget->setStyleSheet(stylesheet);
  }
}

void Appearance::SetThemeMode(ThemeMode mode) {
  theme_mode_ = mode;
  ApplyTheme();
}

void Appearance::LoadUserTheme() { ApplyTheme(); }

void Appearance::ResetToSystemDefaultTheme() {
  QApplication::setPalette(BasePalette());
}

void Appearance::ChangeForegroundColor(const QColor& color) {
  // Get the application palette
  QPalette p = QApplication::palette();

  // Modify the palette
  p.setColor(QPalette::WindowText, color);
  p.setColor(QPalette::Text, color);

  // Make the modified palette the new application's palette
  QApplication::setPalette(p);
  foreground_color_ = color;
}

void Appearance::ChangeBackgroundColor(const QColor& color) {
  // Get the application palette
  QPalette p = QApplication::palette();

  // Modify the palette
  p.setColor(QPalette::Window, color);
  p.setColor(QPalette::Base, color);

  // Make the modified palette the new application's palette
  QApplication::setPalette(p);
  background_color_ = color;
}
