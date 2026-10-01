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

#include "songinfo/lyricsfetcher.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>

#include "core/timeconstants.h"
#include "test_utils.h"

namespace {

class LyricsFetcherTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_TRUE(dir_.isValid());
    song_.Init("Creep", "Radiohead", "Pablo Honey", 238 * kNsecPerSec);
    song_.set_url(QUrl::fromLocalFile(dir_.filePath("02 Creep.flac")));
  }

  void Write(const QString& name, const QByteArray& data) {
    QFile file(dir_.filePath(name));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(data);
  }

  QTemporaryDir dir_;
  Song song_;
};

TEST_F(LyricsFetcherTest, NothingLocal) {
  EXPECT_TRUE(LyricsFetcher::FromLocal(song_).IsEmpty());
}

TEST_F(LyricsFetcherTest, TagsFirst) {
  Write("02 Creep.lrc", "[00:01.00]From the file");
  song_.set_lyrics("From the tags");
  Lyrics lyrics = LyricsFetcher::FromLocal(song_);
  EXPECT_EQ("From the tags", lyrics.plain);
  EXPECT_TRUE(lyrics.lrc.isEmpty());
}

TEST_F(LyricsFetcherTest, TimedTags) {
  song_.set_lyrics("[00:01.00]When you were here before\n[00:05.00]Couldn't");
  Lyrics lyrics = LyricsFetcher::FromLocal(song_);
  EXPECT_EQ("When you were here before\nCouldn't", lyrics.plain);
  EXPECT_EQ(song_.lyrics(), lyrics.lrc);
}

TEST_F(LyricsFetcherTest, LrcFile) {
  Write("02 Creep.txt", "Plain");
  Write("02 Creep.lrc", "[00:01.00]Timed\n");
  Lyrics lyrics = LyricsFetcher::FromLocal(song_);
  EXPECT_EQ("Timed", lyrics.plain);
  EXPECT_FALSE(lyrics.lrc.isEmpty());
  EXPECT_TRUE(lyrics.title.contains("02 Creep.lrc"));
}

TEST_F(LyricsFetcherTest, TxtFile) {
  Write("02 Creep.txt", "\xef\xbb\xbfWhen you were here before\n");
  Lyrics lyrics = LyricsFetcher::FromLocal(song_);
  EXPECT_EQ("When you were here before", lyrics.plain);
  EXPECT_TRUE(lyrics.lrc.isEmpty());
}

TEST_F(LyricsFetcherTest, NotUtf8) {
  Write("02 Creep.txt", "Caf\xe9");
  EXPECT_FALSE(LyricsFetcher::FromLocal(song_).IsEmpty());
}

TEST_F(LyricsFetcherTest, OtherFilesIgnored) {
  Write("03 Other.lrc", "[00:01.00]Someone else's");
  Write("02 Creep.flac.lrc", "[00:01.00]Wrong name");
  EXPECT_TRUE(LyricsFetcher::FromLocal(song_).IsEmpty());
}

TEST_F(LyricsFetcherTest, NoFilesForCueTracks) {
  Write("02 Creep.lrc", "[00:01.00]The whole CUE's file");
  song_.set_cue_path(dir_.filePath("album.cue"));
  EXPECT_TRUE(LyricsFetcher::FromLocal(song_).IsEmpty());
}

QJsonArray Results(const char* json) {
  return QJsonDocument::fromJson(json).array();
}

TEST_F(LyricsFetcherTest, ChoosesClosestLength) {
  QJsonObject chosen = LyricsFetcher::ChooseSearchResult(Results(R"json([
      {"id": 1, "artistName": "Radiohead", "trackName": "Creep",
       "duration": 247, "plainLyrics": "Live"},
      {"id": 2, "artistName": "RADIOHEAD", "trackName": "Creep",
       "duration": 239, "plainLyrics": "Album"},
      {"id": 3, "artistName": "Radiohead", "trackName": "Creep",
       "duration": 300, "plainLyrics": "Too long"}])json"),
                                                         song_);
  EXPECT_EQ(2, chosen["id"].toInt());
}

TEST_F(LyricsFetcherTest, RejectsOtherSongs) {
  const QJsonArray results = Results(R"json([
      {"artistName": "Stone Temple Pilots", "trackName": "Creep",
       "duration": 238, "plainLyrics": "Other artist"},
      {"artistName": "Radiohead", "trackName": "Creep (Acoustic)",
       "duration": 238, "plainLyrics": "Other version"},
      {"artistName": "Radiohead", "trackName": "Creep",
       "duration": 238, "plainLyrics": null, "syncedLyrics": null},
      {"artistName": "Radiohead", "trackName": "Creep",
       "duration": 260, "plainLyrics": "Too far"}])json");
  ASSERT_EQ(4, results.size());
  EXPECT_TRUE(LyricsFetcher::ChooseSearchResult(results, song_).isEmpty());
}

TEST_F(LyricsFetcherTest, MatchesPunctuationAndInstrumentals) {
  song_.Init("Dog Eat Dog", "AC/DC", "", 0);
  QJsonObject chosen = LyricsFetcher::ChooseSearchResult(Results(R"json([
      {"id": 7, "artistName": "ACDC", "trackName": "Dog Eat Dog",
       "duration": 215, "instrumental": true}])json"),
                                                         song_);
  EXPECT_EQ(7, chosen["id"].toInt());
}

}  // namespace
