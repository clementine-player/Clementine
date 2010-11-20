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

#include "gtest/gtest.h"
#include "test_utils.h"
#include "core/mergedproxymodel.h"

#include <QStandardItemModel>
#include <QSignalSpy>

class MergedProxyModelTest : public ::testing::Test {
 protected:
  void SetUp() {
    merged_.setSourceModel(&source_);
  }

  QStandardItemModel source_;
  MergedProxyModel merged_;
};

TEST_F(MergedProxyModelTest, Flat) {
  source_.appendRow(new QStandardItem("one"));
  source_.appendRow(new QStandardItem("two"));

  ASSERT_EQ(2, merged_.rowCount(QModelIndex()));
  QModelIndex one_i = merged_.index(0, 0, QModelIndex());
  QModelIndex two_i = merged_.index(1, 0, QModelIndex());

  EXPECT_EQ("one", one_i.data().toString());
  EXPECT_EQ("two", two_i.data().toString());
  EXPECT_FALSE(merged_.parent(one_i).isValid());
  EXPECT_FALSE(merged_.hasChildren(one_i));
}

TEST_F(MergedProxyModelTest, Tree) {
  QStandardItem* one = new QStandardItem("one");
  QStandardItem* two = new QStandardItem("two");
  source_.appendRow(one);
  one->appendRow(two);

  ASSERT_EQ(1, merged_.rowCount(QModelIndex()));
  QModelIndex one_i = merged_.index(0, 0, QModelIndex());

  ASSERT_EQ(1, merged_.rowCount(one_i));
  QModelIndex two_i = merged_.index(0, 0, one_i);

  EXPECT_EQ("one", one_i.data().toString());
  EXPECT_EQ("two", two_i.data().toString());
  EXPECT_EQ("one", two_i.parent().data().toString());
}

TEST_F(MergedProxyModelTest, Merged) {
  source_.appendRow(new QStandardItem("one"));

  QStandardItemModel submodel;
  submodel.appendRow(new QStandardItem("two"));

  merged_.AddSubModel(source_.index(0, 0, QModelIndex()), &submodel);

  ASSERT_EQ(1, merged_.rowCount(QModelIndex()));
  QModelIndex one_i = merged_.index(0, 0, QModelIndex());

  EXPECT_EQ("one", merged_.data(one_i).toString());
  EXPECT_TRUE(merged_.hasChildren(one_i));

  ASSERT_EQ(1, merged_.rowCount(one_i));
  QModelIndex two_i = merged_.index(0, 0, one_i);

  EXPECT_EQ("two", merged_.data(two_i).toString());
  EXPECT_EQ(0, merged_.rowCount(two_i));
  EXPECT_FALSE(merged_.hasChildren(two_i));
}

TEST_F(MergedProxyModelTest, SourceInsert) {
  QSignalSpy before_spy(&merged_, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
  QSignalSpy after_spy(&merged_, SIGNAL(rowsInserted(QModelIndex,int,int)));

  source_.appendRow(new QStandardItem("one"));

  ASSERT_EQ(1, before_spy.count());
  ASSERT_EQ(1, after_spy.count());
  EXPECT_FALSE(before_spy[0][0].value<QModelIndex>().isValid());
  EXPECT_EQ(0, before_spy[0][1].toInt());
  EXPECT_EQ(0, before_spy[0][2].toInt());
  EXPECT_FALSE(after_spy[0][0].value<QModelIndex>().isValid());
  EXPECT_EQ(0, after_spy[0][1].toInt());
  EXPECT_EQ(0, after_spy[0][2].toInt());
}

TEST_F(MergedProxyModelTest, SourceRemove) {
  source_.appendRow(new QStandardItem("one"));

  QSignalSpy before_spy(&merged_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy after_spy(&merged_, SIGNAL(rowsRemoved(QModelIndex,int,int)));

  source_.removeRow(0, QModelIndex());

  ASSERT_EQ(1, before_spy.count());
  ASSERT_EQ(1, after_spy.count());
  EXPECT_FALSE(before_spy[0][0].value<QModelIndex>().isValid());
  EXPECT_EQ(0, before_spy[0][1].toInt());
  EXPECT_EQ(0, before_spy[0][2].toInt());
  EXPECT_FALSE(after_spy[0][0].value<QModelIndex>().isValid());
  EXPECT_EQ(0, after_spy[0][1].toInt());
  EXPECT_EQ(0, after_spy[0][2].toInt());
}

TEST_F(MergedProxyModelTest, SubInsert) {
  source_.appendRow(new QStandardItem("one"));
  QStandardItemModel submodel;
  merged_.AddSubModel(source_.index(0, 0, QModelIndex()), &submodel);

  QSignalSpy before_spy(&merged_, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
  QSignalSpy after_spy(&merged_, SIGNAL(rowsInserted(QModelIndex,int,int)));

  submodel.appendRow(new QStandardItem("two"));

  ASSERT_EQ(1, before_spy.count());
  ASSERT_EQ(1, after_spy.count());
  EXPECT_EQ("one", before_spy[0][0].value<QModelIndex>().data());
  EXPECT_EQ(0, before_spy[0][1].toInt());
  EXPECT_EQ(0, before_spy[0][2].toInt());
  EXPECT_EQ("one", after_spy[0][0].value<QModelIndex>().data());
  EXPECT_EQ(0, after_spy[0][1].toInt());
  EXPECT_EQ(0, after_spy[0][2].toInt());
}

TEST_F(MergedProxyModelTest, SubRemove) {
  source_.appendRow(new QStandardItem("one"));
  QStandardItemModel submodel;
  merged_.AddSubModel(source_.index(0, 0, QModelIndex()), &submodel);

  submodel.appendRow(new QStandardItem("two"));

  QSignalSpy before_spy(&merged_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy after_spy(&merged_, SIGNAL(rowsRemoved(QModelIndex,int,int)));

  submodel.removeRow(0, QModelIndex());

  ASSERT_EQ(1, before_spy.count());
  ASSERT_EQ(1, after_spy.count());
  EXPECT_EQ("one", before_spy[0][0].value<QModelIndex>().data());
  EXPECT_EQ(0, before_spy[0][1].toInt());
  EXPECT_EQ(0, before_spy[0][2].toInt());
  EXPECT_EQ("one", after_spy[0][0].value<QModelIndex>().data());
  EXPECT_EQ(0, after_spy[0][1].toInt());
  EXPECT_EQ(0, after_spy[0][2].toInt());
}
