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

#include "syncedlyricsview.h"

#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <algorithm>

#include "songinfo/lrcparser.h"

SyncedLyricsView::SyncedLyricsView(const QString& lrc, QWidget* parent)
    : SongInfoTextView(parent), current_(-1) {
  QStringList lines;
  for (const LrcParser::Line& line : LrcParser::Parse(lrc)) {
    // Blank lines mark gaps: keep one between verses, not a run of them.
    if (line.text.isEmpty() && (lines.isEmpty() || lines.last().isEmpty())) {
      continue;
    }
    lines << line.text;
    times_ << line.time_msec;
  }
  while (!lines.isEmpty() && lines.last().isEmpty()) {
    lines.removeLast();
    times_.removeLast();
  }

  // One block per line, so a line's block number is its index.
  setPlainText(lines.join('\n'));
  FormatAll();
}

void SyncedLyricsView::SetPosition(qint64 msec) {
  // The last line that's started.
  const int line = int(std::upper_bound(times_.begin(), times_.end(), msec) -
                       times_.begin()) -
                   1;
  if (line == current_) return;

  if (current_ >= 0) FormatLine(current_, false);
  current_ = line;
  if (current_ >= 0) FormatLine(current_, true);

  // A bold line might wrap where it didn't. (Once it's laid out: as in
  // SongInfoTextView::resizeEvent().)
  const int height = document()->size().height();
  if (document()->textWidth() > 0 && height != minimumHeight()) {
    setMinimumHeight(height);
  }
  EmitCurrentLine();
}

void SyncedLyricsView::EmitCurrentLine() {
  if (current_ < 0) return;
  const QTextBlock block = document()->findBlockByNumber(current_);
  const QRectF rect = document()->documentLayout()->blockBoundingRect(block);
  emit CurrentLineMoved(viewport()->y() + int(rect.y()), int(rect.height()));
}

void SyncedLyricsView::FormatLine(int line, bool current) {
  // The current line in bold, in the usual colour, and the rest halfway to
  // the background.
  const QColor text = palette().color(QPalette::Text);
  const QColor base = palette().color(QPalette::Base);
  QTextCharFormat format;
  format.setFontWeight(current ? QFont::Bold : QFont::Normal);
  format.setForeground(
      current ? text
              : QColor::fromRgbF((text.redF() + base.redF()) / 2,
                                 (text.greenF() + base.greenF()) / 2,
                                 (text.blueF() + base.blueF()) / 2));

  QTextCursor cursor(document()->findBlockByNumber(line));
  cursor.select(QTextCursor::BlockUnderCursor);
  cursor.mergeCharFormat(format);
}

void SyncedLyricsView::FormatAll() {
  for (int i = 0; i < times_.size(); ++i) FormatLine(i, i == current_);
}

void SyncedLyricsView::changeEvent(QEvent* e) {
  SongInfoTextView::changeEvent(e);
  // A new theme: new colours.
  if (e->type() == QEvent::PaletteChange) FormatAll();
}
