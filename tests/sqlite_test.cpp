#include "gtest/gtest.h"

#include <sqlite3.h>

TEST(SqliteTest, FTS3SupportEnabled) {
  sqlite3* db = NULL;
  int rc = sqlite3_open(":memory:", &db);
  ASSERT_EQ(0, rc);

  char* errmsg = NULL;
  rc = sqlite3_exec(
      db, "CREATE VIRTUAL TABLE foo USING fts3(content, TEXT)",
      NULL, NULL, &errmsg);
  ASSERT_EQ(0, rc) << errmsg;

  sqlite3_close(db);
}
