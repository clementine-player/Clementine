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

#ifndef CORE_APPEARANCE_H_
#define CORE_APPEARANCE_H_

#include <QColor>
#include <QObject>
#include <QPalette>

class Appearance : public QObject {
  Q_OBJECT

 public:
  explicit Appearance(QObject* parent = nullptr);

  // The base palette everything else is built on. System follows the
  // desktop's own preference where Qt is new enough to report it (6.5+), and
  // behaves as Light where it isn't - see EffectiveTheme().
  enum ThemeMode {
    ThemeMode_System = 0,
    ThemeMode_Light = 1,
    ThemeMode_Dark = 2,
  };

  // Load the user preferred theme: the light or dark base palette, with the
  // custom set of colours the user has chosen applied on top of it if they
  // enabled one.
  void LoadUserTheme();
  // Drop any custom colour set and go back to the plain base palette for the
  // current theme. Despite the name this is not necessarily the *system*
  // palette any more - in dark mode the base is our dark palette.
  void ResetToSystemDefaultTheme();
  void ChangeForegroundColor(const QColor& color);
  void ChangeBackgroundColor(const QColor& color);

  ThemeMode theme_mode() const { return theme_mode_; }
  // Applies a theme immediately without saving it, so the settings page can
  // preview one and still put the old theme back if the user cancels.
  void SetThemeMode(ThemeMode mode);

  // theme_mode() with System resolved to whatever the desktop actually asked
  // for, so callers get a straight Light-or-Dark answer.
  ThemeMode EffectiveTheme() const;
  // Whether the palette currently in use is a dark one. Widgets that paint
  // themselves rather than going through the style use this to pick colours,
  // and it deliberately looks at the live palette rather than the setting so
  // that a custom colour set is taken into account too.
  static bool IsDarkPalette(const QPalette& palette);

  static QPalette DarkPalette();
  static QPalette LightPalette();

  static const char* kSettingsGroup;
  static const char* kUseCustomColorSet;
  static const char* kForegroundColor;
  static const char* kBackgroundColor;
  static const char* kThemeMode;
  static const QPalette kDefaultPalette;

 private slots:
  // Any xdg-desktop-portal setting changed. Deliberately takes no arguments
  // so the header doesn't have to pull in the QtDBus types just to be moc'd -
  // we re-read the value rather than trusting what came with the signal.
  void DesktopSettingChanged();

 private:
  // The unmodified palette for the current theme, before any custom colour
  // set is layered on: the system palette for light, ours for dark.
  QPalette BasePalette() const;
  void ApplyTheme();
  // Forces stylesheet-styled widgets to re-resolve against the new palette.
  void RepolishStyledWidgets();
  // Re-reads the desktop's colour scheme preference into desktop_scheme_.
  void RefreshDesktopScheme();

  ThemeMode theme_mode_;
  // What the desktop says it wants, or ThemeMode_System when it hasn't told
  // us. Cached rather than queried per call because reading it goes over
  // D-Bus, and EffectiveTheme() is called on every repaint path.
  ThemeMode desktop_scheme_;
  QColor foreground_color_;
  QColor background_color_;
};

#endif  // CORE_APPEARANCE_H_
