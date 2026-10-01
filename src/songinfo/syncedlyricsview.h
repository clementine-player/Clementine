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

#ifndef SONGINFO_SYNCEDLYRICSVIEW_H_
#define SONGINFO_SYNCEDLYRICSVIEW_H_

#include <QList>

#include "songinfo/songinfotextview.h"

// Timed (LRC) lyrics, with the line being sung highlighted and the rest
// dimmed. Something else tells it where the song is, with SetPosition().
class SyncedLyricsView : public SongInfoTextView {
  Q_OBJECT

 public:
  explicit SyncedLyricsView(const QString& lrc, QWidget* parent = nullptr);

  // Whether the lyrics had any timed lines to show.
  bool has_lines() const { return !times_.isEmpty(); }
  // The line being sung, or -1 before the first.
  int current_line() const { return current_; }

  void SetPosition(qint64 msec);
  // Emits CurrentLineMoved() for the current line, if there is one.
  void EmitCurrentLine();

 signals:
  // The current line moved to here, in this widget's coordinates.
  void CurrentLineMoved(int y, int height);

 protected:
  void changeEvent(QEvent* e) override;

 private:
  void FormatLine(int line, bool current);
  void FormatAll();

  // When each line (each block of the document) starts.
  QList<qint64> times_;
  int current_;
};

#endif  // SONGINFO_SYNCEDLYRICSVIEW_H_
