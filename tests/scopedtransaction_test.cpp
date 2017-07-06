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

#include "test_utils.h"
#include "gtest/gtest.h"

#include "core/scopedtransaction.h"

#include <QtDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

namespace {

class ScopedTransactionTest : public ::testing::Test {
 protected:
  void SetUp() {
    database_ = QSqlDatabase::addDatabase("QSQLITE");
    database_.setDatabaseName(":memory:");
    ASSERT_TRUE(database_.open());
  }

  void TearDown() {
    // Make sure Qt does not re-use the connection.
    QString name = database_.connectionName();
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
  }

  QSqlDatabase database_;
};

TEST_F(ScopedTransactionTest, OpensATransaction) {
  // False because there is no transaction to roll back
  EXPECT_FALSE(database_.rollback());

  ScopedTransaction t(&database_);

  // There should now be a transaction
  EXPECT_TRUE(database_.rollback());
}

TEST_F(ScopedTransactionTest, RollbackOnDtor) {
  database_.exec("CREATE TABLE foo (bar INTEGER)");

  {
    ScopedTransaction t(&database_);
    database_.exec("INSERT INTO foo (bar) VALUES (42)");

    QSqlQuery q(database_);
    q.prepare("SELECT * FROM foo");
    ASSERT_TRUE(q.exec());
    ASSERT_TRUE(q.next());
    EXPECT_EQ(42, q.value(0).toInt());
  }

  QSqlQuery q(database_);
  q.prepare("SELECT * FROM foo");
  ASSERT_TRUE(q.exec());
  ASSERT_FALSE(q.next());
}

TEST_F(ScopedTransactionTest, Commit) {
  database_.exec("CREATE TABLE foo (bar INTEGER)");

  {
    ScopedTransaction t(&database_);
    database_.exec("INSERT INTO foo (bar) VALUES (42)");
    t.Commit();
  }

  QSqlQuery q(database_);
  q.prepare("SELECT * FROM foo");
  ASSERT_TRUE(q.exec());
  ASSERT_TRUE(q.next());
  EXPECT_EQ(42, q.value(0).toInt());
}


} // namespace
