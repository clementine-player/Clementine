/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef SONGINFO_LRCPARSER_H_
#define SONGINFO_LRCPARSER_H_

#include <QList>
#include <QString>

// Lyrics in LRC format, where each line starts with the time it's sung:
//   [00:12.34]Words
// A line can have several times ("[00:12.34][01:02.00]Chorus"), and there
// can be tags ("[ar:Artist]", "[offset:+250]") and word times
// ("<00:12.50>Words").
class LrcParser {
 public:
  struct Line {
    qint64 time_msec;
    QString text;
  };

  // Whether the text has at least one timed line.
  static bool IsLrc(const QString& text);

  // The timed lines, in the order they're sung.
  static QList<Line> Parse(const QString& text);

  // The words without their times or tags, in the order they're sung.
  static QString ToPlainText(const QString& text);
};

#endif  // SONGINFO_LRCPARSER_H_
