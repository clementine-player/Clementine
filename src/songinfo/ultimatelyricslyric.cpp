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

#include "ultimatelyricslyric.h"

#include <QRegExp>

UltimateLyricsLyric::UltimateLyricsLyric(QObject* parent) :
  QTextDocument(parent) {
}

void UltimateLyricsLyric::SetHtml(const QString& html) {
  QString copy(html.trimmed());

  // Simplify newlines, and convert them to <p>
  copy.replace(QRegExp("[\\r\\n]+"), "\n");
  copy.replace(QRegExp("([^>])[\\t ]*\\n"), "\\1<p>");

  // Strip any newlines from the end
  copy.replace(QRegExp("((<\\s*br\\s*/?\\s*>)|(<\\s*/?\\s*p\\s*/?\\s*>))+$"), "");

  setHtml(copy);
}
