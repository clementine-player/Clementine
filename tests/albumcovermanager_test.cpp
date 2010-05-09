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

#include "albumcovermanager.h"

#include "gtest/gtest.h"

#include "mock_networkaccessmanager.h"

class AlbumCoverManagerTest : public ::testing::Test {
 protected:
  AlbumCoverManagerTest()
      : manager_(&network_, NULL) {
  }

  MockNetworkAccessManager network_;
  AlbumCoverManager manager_;
};

TEST_F(AlbumCoverManagerTest, HidesItemsWithCover) {
  QListWidgetItem hidden_item;
  EXPECT_TRUE(manager_.ShouldHide(hidden_item, QString(), AlbumCoverManager::Hide_WithCovers));
  QListWidgetItem shown_item;
  shown_item.setIcon(manager_.no_cover_icon_);
  EXPECT_FALSE(manager_.ShouldHide(shown_item, QString(), AlbumCoverManager::Hide_WithCovers));
}

TEST_F(AlbumCoverManagerTest, HidesItemsWithoutCover) {
  QListWidgetItem hidden_item;
  hidden_item.setIcon(manager_.no_cover_icon_);
  EXPECT_TRUE(manager_.ShouldHide(hidden_item, QString(), AlbumCoverManager::Hide_WithoutCovers));
  QListWidgetItem shown_item;
  EXPECT_FALSE(manager_.ShouldHide(shown_item, QString(), AlbumCoverManager::Hide_WithoutCovers));
}

TEST_F(AlbumCoverManagerTest, HidesItemsWithFilter) {
  QListWidgetItem hidden_item;
  hidden_item.setText("barbaz");
  EXPECT_TRUE(manager_.ShouldHide(hidden_item, "foo", AlbumCoverManager::Hide_None));
  EXPECT_TRUE(manager_.ShouldHide(hidden_item, "foo abc", AlbumCoverManager::Hide_None));
  QListWidgetItem shown_item;
  shown_item.setText("foobar");
  EXPECT_FALSE(manager_.ShouldHide(shown_item, "foo", AlbumCoverManager::Hide_None));
  EXPECT_TRUE(manager_.ShouldHide(shown_item, "abc bar", AlbumCoverManager::Hide_None));
  EXPECT_FALSE(manager_.ShouldHide(shown_item, "bar foo", AlbumCoverManager::Hide_None));
}
