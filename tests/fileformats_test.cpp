/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

// Reads the tags of a short file in each format Clementine plays, with real
// tag reader workers.

#include <QDir>
#include <QFile>
#include <QTemporaryFile>

#include "core/song.h"
#include "core/tagreaderclient.h"
#include "gtest/gtest.h"
#include "tagreader_env.h"
#include "test_utils.h"

namespace {

::testing::Environment* const kTagReader =
    ::testing::AddGlobalTestEnvironment(new TagReaderEnvironment);

class FileformatsTest : public ::testing::TestWithParam<const char*> {
 protected:
  void SetUp() {
    format_ = GetParam();
    resource_filename_ = ":/testdata/beep." + format_;
  }

  QString format_;
  QString resource_filename_;
};

TEST_P(FileformatsTest, Exists) {
  EXPECT_TRUE(QFile::exists(resource_filename_));
}

TEST_P(FileformatsTest, LoadsTags) {
  // With the format's extension, which the tag reader goes by.
  QTemporaryFile temp(QDir::tempPath() + "/fileformatstest-XXXXXX." + format_);
  ASSERT_TRUE(temp.open());
  QFile resource(resource_filename_);
  ASSERT_TRUE(resource.open(QIODevice::ReadOnly));
  temp.write(resource.readAll());
  temp.flush();

  Song song;
  TagReaderClient::Instance()->ReadFileBlocking(temp.fileName(), &song);
  ASSERT_TRUE(song.is_valid());

  // WAV files have no tags.
  if (format_ != "wav") {
    EXPECT_EQ("Beep " + format_, song.title());
  }
}

INSTANTIATE_TEST_CASE_P(Formats, FileformatsTest,
                        ::testing::Values("flac", "mp3", "ogg", "spx", "wav",
                                          "wma", "m4a"));

}  // namespace
