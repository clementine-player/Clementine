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

#include "test_utils.h"
#include "gtest/gtest.h"

#include "playlist.h"
#include "mock_settingsprovider.h"

#include <QtDebug>

#include <boost/scoped_ptr.hpp>

namespace {

class PlaylistTest : public ::testing::Test {
 protected:
  PlaylistTest()
    : playlist_(new Playlist(NULL, new DummySettingsProvider)),
      sequence_(new PlaylistSequence(NULL, new DummySettingsProvider))
  {
  }

  void SetUp() {
    playlist_->set_sequence(sequence_.get());
  }

  boost::scoped_ptr<Playlist> playlist_;
  boost::scoped_ptr<PlaylistSequence> sequence_;
};

TEST_F(PlaylistTest, Basic) {
  EXPECT_EQ(0, playlist_->rowCount(QModelIndex()));
}

}

