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

#ifndef UI_SYMBOLICICON_H
#define UI_SYMBOLICICON_H

#include <QIcon>
#include <QString>

// Whether to draw the common actions with the icon theme's symbolic icons:
// on under GNOME, where they're the native look. CLEMENTINE_SYMBOLIC_ICONS=1
// or 0 turns them on or off anywhere.
bool UseSymbolicIcons();

// The symbolic icon name (without "-symbolic") for one of Clementine's
// freedesktop-style icon names, or an empty string if there isn't a good one.
QString SymbolicNameForIconName(const QString& icon_name);

// The icon theme's "<name>-symbolic" icon, recoloured whenever it's painted
// with the palette's text colour at the time, the way GTK draws them, so it
// follows light and dark. A null icon if the theme doesn't have it.
QIcon SymbolicIcon(const QString& name);

#endif  // UI_SYMBOLICICON_H
