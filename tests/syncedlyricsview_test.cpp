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

#include "songinfo/syncedlyricsview.h"

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTextBlock>

#include "test_utils.h"

namespace {

const char* kLrc =
    "[ar:Artist]\n"
    "[00:00.00]\n"
    "[00:01.00]One\n"
    "[00:02.00]Two\n"
    "[00:03.00]\n"
    "[00:03.50]\n"
    "[00:04.00][00:06.00]Chorus\n"
    "[00:05.00]Three\n"
    "[00:07.00]\n";

QColor LineColour(const SyncedLyricsView& view, int line) {
  QTextBlock block = view.document()->findBlockByNumber(line);
  return block.begin().fragment().charFormat().foreground().color();
}

TEST(SyncedLyricsViewTest, Lines) {
  SyncedLyricsView view(kLrc);
  EXPECT_TRUE(view.has_lines());
  // Leading and trailing gaps go, and a run of them is one.
  EXPECT_EQ(QStringList({"One", "Two", "", "Chorus", "Three", "Chorus"}),
            view.toPlainText().split('\n'));
  EXPECT_EQ(-1, view.current_line());
}

TEST(SyncedLyricsViewTest, NoTimedLines) {
  SyncedLyricsView view("[ar:Artist]\nJust words");
  EXPECT_FALSE(view.has_lines());
}

TEST(SyncedLyricsViewTest, FollowsPosition) {
  SyncedLyricsView view(kLrc);
  QSignalSpy moved(&view, &SyncedLyricsView::CurrentLineMoved);

  // Before the first line.
  view.SetPosition(500);
  EXPECT_EQ(-1, view.current_line());
  EXPECT_EQ(0, moved.count());

  view.SetPosition(1500);
  EXPECT_EQ(0, view.current_line());
  EXPECT_EQ(1, moved.count());

  // Same line: nothing to do.
  view.SetPosition(1900);
  EXPECT_EQ(1, moved.count());

  view.SetPosition(4000);
  EXPECT_EQ(3, view.current_line());
  view.SetPosition(6500);
  EXPECT_EQ(5, view.current_line());

  // Seeking back.
  view.SetPosition(2000);
  EXPECT_EQ(1, view.current_line());
  EXPECT_EQ(4, moved.count());

  // Lower lines are further down, once it's laid out, as it is when shown.
  view.document()->setTextWidth(300);
  view.SetPosition(1000);
  const int one = moved.last().at(0).toInt();
  view.SetPosition(5000);
  EXPECT_GT(moved.last().at(0).toInt(), one);
}

TEST(SyncedLyricsViewTest, HighlightsCurrentLine) {
  SyncedLyricsView view(kLrc);
  const QColor text = view.palette().color(QPalette::Text);

  view.SetPosition(2000);
  EXPECT_EQ(text, LineColour(view, 1));
  EXPECT_NE(text, LineColour(view, 0));
  EXPECT_EQ(QFont::Bold, view.document()
                             ->findBlockByNumber(1)
                             .begin()
                             .fragment()
                             .charFormat()
                             .fontWeight());

  view.SetPosition(5000);
  EXPECT_EQ(text, LineColour(view, 4));
  EXPECT_NE(text, LineColour(view, 1));
}

}  // namespace
