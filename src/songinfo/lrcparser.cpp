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

#include "lrcparser.h"

#include <QRegularExpression>
#include <algorithm>

namespace {

// One or more "[mm:ss]", "[mm:ss.xx]" or "[mm:ss:xx]" at the start of a line.
const QRegularExpression kTimesRe(
    R"(^\s*((?:\[\d+:\d{1,2}(?:[.:]\d{1,3})?\])+)(.*)$)");
const QRegularExpression kTimeRe(R"(\[(\d+):(\d{1,2})(?:[.:](\d{1,3}))?\])");
const QRegularExpression kOffsetRe(R"(^\s*\[offset:\s*([+-]?\d+)\s*\]\s*$)",
                                   QRegularExpression::CaseInsensitiveOption);
const QRegularExpression kWordTimeRe(R"(<\d+:\d{1,2}(?:[.:]\d{1,3})?>)");

qint64 ToMsec(const QRegularExpressionMatch& m) {
  const qint64 minutes = m.captured(1).toLongLong();
  const qint64 seconds = m.captured(2).toLongLong();
  // ".5" is half a second, ".05" five hundredths, ".005" five thousandths.
  QString fraction = m.captured(3);
  qint64 msec = 0;
  if (!fraction.isEmpty()) {
    msec = fraction.leftJustified(3, '0').left(3).toLongLong();
  }
  return (minutes * 60 + seconds) * 1000 + msec;
}

}  // namespace

bool LrcParser::IsLrc(const QString& text) {
  const QStringList lines = text.split('\n');
  return std::any_of(lines.begin(), lines.end(), [](const QString& line) {
    return kTimesRe.match(line).hasMatch();
  });
}

QList<LrcParser::Line> LrcParser::Parse(const QString& text) {
  QList<Line> ret;
  qint64 offset_msec = 0;

  for (QString line : text.split('\n')) {
    line.remove('\r');

    // A positive offset means the words come sooner.
    QRegularExpressionMatch offset = kOffsetRe.match(line);
    if (offset.hasMatch()) {
      offset_msec = offset.captured(1).toLongLong();
      continue;
    }

    QRegularExpressionMatch times = kTimesRe.match(line);
    if (!times.hasMatch()) continue;

    QString words = times.captured(2);
    words.remove(kWordTimeRe);
    words = words.trimmed();

    QRegularExpressionMatchIterator it = kTimeRe.globalMatch(times.captured(1));
    while (it.hasNext()) {
      ret << Line{ToMsec(it.next()), words};
    }
  }

  for (Line& line : ret) {
    line.time_msec = qMax(0ll, line.time_msec - offset_msec);
  }
  std::stable_sort(ret.begin(), ret.end(), [](const Line& a, const Line& b) {
    return a.time_msec < b.time_msec;
  });
  return ret;
}

QString LrcParser::ToPlainText(const QString& text) {
  QStringList lines;
  for (const Line& line : Parse(text)) {
    // Blank lines mark gaps: keep one between verses, not a run of them.
    if (line.text.isEmpty() && (lines.isEmpty() || lines.last().isEmpty())) {
      continue;
    }
    lines << line.text;
  }
  while (!lines.isEmpty() && lines.last().isEmpty()) lines.removeLast();
  return lines.join('\n');
}
