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

#include "songinfo/songinfobase.h"

#include <gtest/gtest.h>

#include <QLabel>

#include "test_utils.h"

namespace {

// With no providers, so nothing is fetched.
class TestInfoView : public SongInfoBase {
 public:
  TestInfoView() { SetPlaceholderText("Nothing playing"); }

  QLabel* placeholder() const {
    for (QLabel* label : findChildren<QLabel*>()) {
      if (label->text() == "Nothing playing") return label;
    }
    return nullptr;
  }
};

Song MakeSong(const QString& title) {
  Song song;
  song.Init(title, "Artist", "Album", 100);
  return song;
}

TEST(SongInfoBaseTest, PlaceholderWhenNothingPlays) {
  TestInfoView view;
  view.show();
  ASSERT_TRUE(view.placeholder());
  EXPECT_TRUE(view.placeholder()->isVisible());

  view.SongChanged(MakeSong("One"));
  EXPECT_FALSE(view.placeholder()->isVisible());

  // Stopped.
  view.SongFinished();
  EXPECT_TRUE(view.placeholder()->isVisible());

  // The same song again is shown again.
  view.SongChanged(MakeSong("One"));
  EXPECT_FALSE(view.placeholder()->isVisible());
}

TEST(SongInfoBaseTest, HiddenViewCatchesUpWhenShown) {
  TestInfoView view;
  view.SongChanged(MakeSong("One"));
  view.show();
  EXPECT_FALSE(view.placeholder()->isVisible());
}

}  // namespace
