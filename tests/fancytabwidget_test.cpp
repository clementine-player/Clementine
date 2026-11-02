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

#include "widgets/fancytabwidget.h"

#include <QIcon>
#include <QLabel>

#include "gtest/gtest.h"

namespace {

class FancyTabWidgetTest : public ::testing::Test {
 protected:
  // The tab widget owns the pages once they're added.
  void SetUp() override {
    tabs_.addTab(search_, QIcon(), "Search");
    tabs_.addTab(library_, QIcon(), "Library");
    tabs_.addTab(files_, QIcon(), "Files");
  }

  FancyTabWidget tabs_;
  QLabel* search_ = new QLabel;
  QLabel* library_ = new QLabel;
  QLabel* files_ = new QLabel;
};

TEST_F(FancyTabWidgetTest, IndexOfPageFindsWrappedPages) {
  EXPECT_EQ(0, tabs_.IndexOfPage(search_));
  EXPECT_EQ(1, tabs_.IndexOfPage(library_));
  EXPECT_EQ(2, tabs_.IndexOfPage(files_));

  QLabel elsewhere;
  EXPECT_EQ(-1, tabs_.IndexOfPage(&elsewhere));
}

// The main window shows the search tab this way for "Search for artist" and
// "Search for album" in the playlist's menu.
TEST_F(FancyTabWidgetTest, SetCurrentWidgetShowsThePagesTab) {
  tabs_.setCurrentIndex(1);
  ASSERT_EQ(1, tabs_.currentIndex());

  tabs_.setCurrentWidget(search_);
  EXPECT_EQ(0, tabs_.currentIndex());

  tabs_.setCurrentWidget(files_);
  EXPECT_EQ(2, tabs_.currentIndex());
}

TEST_F(FancyTabWidgetTest, SetCurrentWidgetIgnoresOtherWidgets) {
  tabs_.setCurrentIndex(1);

  QLabel elsewhere;
  tabs_.setCurrentWidget(&elsewhere);
  EXPECT_EQ(1, tabs_.currentIndex());
}

}  // namespace
