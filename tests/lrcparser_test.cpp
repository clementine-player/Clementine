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

#include "songinfo/lrcparser.h"

#include <gtest/gtest.h>

#include "test_utils.h"

namespace {

TEST(LrcParserTest, RecognisesLrc) {
  EXPECT_TRUE(LrcParser::IsLrc("[00:12.34]Words"));
  EXPECT_TRUE(LrcParser::IsLrc("[ar:Artist]\n[00:01]Words"));
  EXPECT_FALSE(LrcParser::IsLrc("Just words\n[Chorus]\nMore words"));
  EXPECT_FALSE(LrcParser::IsLrc("[ar:Artist]\n[ti:Title]"));
  EXPECT_FALSE(LrcParser::IsLrc(""));
}

TEST(LrcParserTest, ParsesTimes) {
  QList<LrcParser::Line> lines = LrcParser::Parse(
      "[ti:Title]\n"
      "[00:01.5]Half\n"
      "[00:02.05]Hundredths\n"
      "[00:03.005]Thousandths\n"
      "[01:04]Whole\n"
      "[00:05:25]Colon\r\n");
  ASSERT_EQ(5, lines.size());
  EXPECT_EQ(1500, lines[0].time_msec);
  EXPECT_EQ("Half", lines[0].text);
  EXPECT_EQ(2050, lines[1].time_msec);
  EXPECT_EQ(3005, lines[2].time_msec);
  EXPECT_EQ(5250, lines[3].time_msec);
  EXPECT_EQ("Colon", lines[3].text);
  EXPECT_EQ(64000, lines[4].time_msec);
}

TEST(LrcParserTest, RepeatedLinesInTimeOrder) {
  QList<LrcParser::Line> lines = LrcParser::Parse(
      "[00:10.00][00:30.00]Chorus\n"
      "[00:20.00]Verse\n");
  ASSERT_EQ(3, lines.size());
  EXPECT_EQ("Chorus", lines[0].text);
  EXPECT_EQ("Verse", lines[1].text);
  EXPECT_EQ("Chorus", lines[2].text);
  EXPECT_EQ(30000, lines[2].time_msec);
}

TEST(LrcParserTest, Offset) {
  // A positive offset means the words come sooner.
  QList<LrcParser::Line> lines =
      LrcParser::Parse("[offset:+500]\n[00:10.00]Sooner\n[00:00.20]Not < 0");
  ASSERT_EQ(2, lines.size());
  EXPECT_EQ(0, lines[0].time_msec);
  EXPECT_EQ(9500, lines[1].time_msec);
}

TEST(LrcParserTest, WordTimes) {
  QList<LrcParser::Line> lines =
      LrcParser::Parse("[00:01.00]<00:01.00>One <00:01.50>two");
  ASSERT_EQ(1, lines.size());
  EXPECT_EQ("One two", lines[0].text);
}

TEST(LrcParserTest, PlainText) {
  EXPECT_EQ("First\nSecond\n\nThird",
            LrcParser::ToPlainText("[ar:Artist]\n"
                                   "[00:00.00]\n"
                                   "[00:01.00]First\n"
                                   "[00:02.00]Second\n"
                                   "[00:03.00]\n"
                                   "[00:03.50]\n"
                                   "[00:04.00]Third\n"
                                   "[00:05.00]\n"));
}

}  // namespace
