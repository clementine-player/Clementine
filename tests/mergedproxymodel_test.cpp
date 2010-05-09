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

#include "gtest/gtest.h"
#include "test_utils.h"
#include "mergedproxymodel.h"

#include <QStandardItemModel>

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

  merged_.AddModel(source_.index(0, 0, QModelIndex()), &submodel);

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
