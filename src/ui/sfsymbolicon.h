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

#ifndef UI_SFSYMBOLICON_H
#define UI_SFSYMBOLICON_H

#include <QIcon>
#include <QString>

// macOS only. The SF Symbol for one of Clementine's freedesktop-style icon
// names, or an empty string if there isn't a good one.
QString SFSymbolForIconName(const QString& icon_name);

// An icon drawn from an SF Symbol whenever it's painted, in the palette's
// text colour at the time: sharp at any size and scale, and following light
// and dark. A null icon if this macOS doesn't have the symbol.
QIcon SFSymbolIcon(const QString& symbol_name);

#endif  // UI_SFSYMBOLICON_H
