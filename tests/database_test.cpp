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

#include "core/database.h"

#include <boost/scoped_ptr.hpp>

#include <QtDebug>
#include <QSqlQuery>
#include <QVariant>

class DatabaseTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    database_.reset(new MemoryDatabase);
  }

  boost::scoped_ptr<Database> database_;
};

#ifdef Q_OS_UNIX

#include <sys/time.h>
#include <time.h>

struct PerfTimer {
  PerfTimer(int iterations) : iterations_(iterations) {
    gettimeofday(&start_time_, NULL);
  }

  ~PerfTimer() {
    gettimeofday(&end_time_, NULL);

    timeval elapsed_time;
    timersub(&end_time_, &start_time_, &elapsed_time);
    int elapsed_us = elapsed_time.tv_usec + elapsed_time.tv_sec * 1000000;

    qDebug() << "Elapsed:" << elapsed_us << "us";
    qDebug() << "Time per iteration:" << float(elapsed_us) / iterations_ << "us";
  }

  timeval start_time_;
  timeval end_time_;
  int iterations_;
};

TEST_F(DatabaseTest, LikePerformance) {
  const int iterations = 1000000;

  const char* needle = "foo";
  const char* haystack = "foobarbaz foobarbaz";
  qDebug() << "Simple query";
  {
    PerfTimer perf(iterations);
    for (int i = 0; i < iterations; ++i) {
      database_->Like(needle, haystack);
    }
  }
}

#endif

TEST_F(DatabaseTest, DatabaseInitialises) {
  // Check that these tables exist
  QStringList tables = database_->Connect().tables();
  EXPECT_TRUE(tables.contains("songs"));
  EXPECT_TRUE(tables.contains("directories"));
  EXPECT_TRUE(tables.contains("subdirectories"));
  EXPECT_TRUE(tables.contains("playlists"));
  EXPECT_TRUE(tables.contains("playlist_items"));
  ASSERT_TRUE(tables.contains("schema_version"));

  // Check the schema version is correct
  QSqlQuery q("SELECT version FROM schema_version", database_->Connect());
  ASSERT_TRUE(q.exec());
  ASSERT_TRUE(q.next());
  EXPECT_EQ(Database::kSchemaVersion, q.value(0).toInt());
  EXPECT_FALSE(q.next());
}

TEST_F(DatabaseTest, LikeWorksWithAllAscii) {
  EXPECT_TRUE(database_->Like("%ar%", "bar"));
  EXPECT_FALSE(database_->Like("%ar%", "foo"));
}

TEST_F(DatabaseTest, LikeWorksWithUnicode) {
  EXPECT_TRUE(database_->Like("%Снег%", "Снег"));
  EXPECT_FALSE(database_->Like("%Снег%", "foo"));
}

TEST_F(DatabaseTest, LikeAsciiCaseInsensitive) {
  EXPECT_TRUE(database_->Like("%ar%", "BAR"));
  EXPECT_FALSE(database_->Like("%ar%", "FOO"));
}

TEST_F(DatabaseTest, LikeUnicodeCaseInsensitive) {
  EXPECT_TRUE(database_->Like("%снег%", "Снег"));
}

TEST_F(DatabaseTest, LikeCacheInvalidated) {
  EXPECT_TRUE(database_->Like("%foo%", "foobar"));
  EXPECT_FALSE(database_->Like("%baz%", "foobar"));
}

TEST_F(DatabaseTest, LikeQuerySplit) {
  EXPECT_TRUE(database_->Like("%foo bar%", "foobar"));
  EXPECT_FALSE(database_->Like("%foo bar%", "barbaz"));
  EXPECT_FALSE(database_->Like("%foo bar%", "foobaz"));
  EXPECT_FALSE(database_->Like("%foo bar%", "baz"));
}

TEST_F(DatabaseTest, LikeDecomposes) {
  EXPECT_TRUE(database_->Like("%Royksopp%", "Röyksopp"));
  EXPECT_FALSE(database_->Like("%Ryksopp%", "Röyksopp"));
  EXPECT_TRUE(database_->Like("%tiesto%", "DJ Tiësto"));
  EXPECT_FALSE(database_->Like("%tisto%", "DJ Tiësto"));
}

TEST_F(DatabaseTest, FTSOpenParsesSimpleInput) {
  sqlite3_tokenizer_cursor* cursor = NULL;
  Database::FTSOpen(NULL, "foo", 3, &cursor);
  ASSERT_TRUE(cursor);
  Database::UnicodeTokenizerCursor* real_cursor = reinterpret_cast<Database::UnicodeTokenizerCursor*>(cursor);
  QList<Database::Token> tokens = real_cursor->tokens;
  ASSERT_EQ(1, tokens.length());
  EXPECT_EQ(0, real_cursor->position);
  EXPECT_TRUE(real_cursor->current_utf8.isEmpty());

  EXPECT_EQ("foo", tokens[0].token);
  EXPECT_EQ(0, tokens[0].start_offset);
  EXPECT_EQ(3, tokens[0].end_offset);
}

TEST_F(DatabaseTest, FTSOpenParsesUTF8Input) {
  sqlite3_tokenizer_cursor* cursor = NULL;
  Database::FTSOpen(NULL, "Röyksopp", 9, &cursor);
  ASSERT_TRUE(cursor);
  Database::UnicodeTokenizerCursor* real_cursor = reinterpret_cast<Database::UnicodeTokenizerCursor*>(cursor);
  QList<Database::Token> tokens = real_cursor->tokens;
  ASSERT_EQ(1, tokens.length());
  EXPECT_EQ(0, real_cursor->position);
  EXPECT_TRUE(real_cursor->current_utf8.isEmpty());

  EXPECT_EQ("royksopp", tokens[0].token);
  EXPECT_EQ(0, tokens[0].start_offset);
  EXPECT_EQ(9, tokens[0].end_offset);
}

TEST_F(DatabaseTest, FTSOpenParsesMultipleTokens) {
  sqlite3_tokenizer_cursor* cursor = NULL;
  Database::FTSOpen(NULL, "Röyksopp foo", 13, &cursor);
  ASSERT_TRUE(cursor);
  Database::UnicodeTokenizerCursor* real_cursor = reinterpret_cast<Database::UnicodeTokenizerCursor*>(cursor);
  QList<Database::Token> tokens = real_cursor->tokens;
  ASSERT_EQ(2, tokens.length());
  EXPECT_EQ(0, real_cursor->position);
  EXPECT_TRUE(real_cursor->current_utf8.isEmpty());

  EXPECT_EQ("royksopp", tokens[0].token);
  EXPECT_EQ(0, tokens[0].start_offset);
  EXPECT_EQ(9, tokens[0].end_offset);

  EXPECT_EQ("foo", tokens[1].token);
  EXPECT_EQ(10, tokens[1].start_offset);
  EXPECT_EQ(13, tokens[1].end_offset);
}

TEST_F(DatabaseTest, FTSOpenLeavesCyrillicQueries) {
  sqlite3_tokenizer_cursor* cursor = NULL;
  const char* query = "Снег";
  Database::FTSOpen(NULL, query, strlen(query), &cursor);
  ASSERT_TRUE(cursor);
  Database::UnicodeTokenizerCursor* real_cursor = reinterpret_cast<Database::UnicodeTokenizerCursor*>(cursor);
  QList<Database::Token> tokens = real_cursor->tokens;
  ASSERT_EQ(1, tokens.length());
  EXPECT_EQ(0, real_cursor->position);
  EXPECT_TRUE(real_cursor->current_utf8.isEmpty());

  EXPECT_EQ(QString::fromUtf8("снег"), tokens[0].token);
  EXPECT_EQ(0, tokens[0].start_offset);
  EXPECT_EQ(strlen(query), tokens[0].end_offset);
}

TEST_F(DatabaseTest, FTSCursorWorks) {
  sqlite3_tokenizer_cursor* cursor = NULL;
  Database::FTSOpen(NULL, "Röyksopp foo", 13, &cursor);
  ASSERT_TRUE(cursor);
  Database::UnicodeTokenizerCursor* real_cursor = reinterpret_cast<Database::UnicodeTokenizerCursor*>(cursor);

  const char* token;
  int bytes = 0;
  int start_offset = 42;
  int end_offset = 0;
  int position = 42;
  int rc = Database::FTSNext(cursor, &token, &bytes, &start_offset, &end_offset, &position);
  EXPECT_EQ(SQLITE_OK, rc);
  EXPECT_STREQ("royksopp", token);
  EXPECT_EQ(8, bytes);
  EXPECT_EQ(0, start_offset);
  EXPECT_EQ(9, end_offset);
  EXPECT_EQ(0, position);

  rc = Database::FTSNext(cursor, &token, &bytes, &start_offset, &end_offset, &position);
  EXPECT_EQ(rc, SQLITE_OK);
  EXPECT_STREQ("foo", token);
  EXPECT_EQ(3, bytes);
  EXPECT_EQ(10, start_offset);
  EXPECT_EQ(13, end_offset);
  EXPECT_EQ(1, position);

  rc = Database::FTSNext(cursor, &token, &bytes, &start_offset, &end_offset, &position);
  EXPECT_EQ(SQLITE_DONE, rc);
}
