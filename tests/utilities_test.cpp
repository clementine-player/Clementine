
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

#include "core/utilities.h"

#include <QDateTime>
#include <QtDebug>

TEST(UtilitiesTest, HmacFunctions) {
  QString key("key");
  QString data("The quick brown fox jumps over the lazy dog");
  // Test Hmac Md5
  QByteArray result_hash_md5 = Utilities::HmacMd5(key.toLocal8Bit(), data.toLocal8Bit()).toHex();
  bool result_md5 = result_hash_md5 == QString("80070713463e7749b90c2dc24911e275");
  EXPECT_TRUE(result_md5);
  // Test Hmac Sha256
  QByteArray result_hash_sha256 = Utilities::HmacSha256(key.toLocal8Bit(), data.toLocal8Bit()).toHex();
  bool result_sha256 = result_hash_sha256 == QString("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
  EXPECT_TRUE(result_sha256);
}

TEST(UtilitiesTest, ParseRFC822DateTime) {
  QDateTime result_DateTime = Utilities::ParseRFC822DateTime(QString("22 Feb 2008 00:16:17 GMT"));
  EXPECT_TRUE(result_DateTime.isValid());
  result_DateTime = Utilities::ParseRFC822DateTime(QString("Thu, 13 Dec 2012 13:27:52 +0000"));
  EXPECT_TRUE(result_DateTime.isValid());
  result_DateTime = Utilities::ParseRFC822DateTime(QString("Mon, 12 March 2012 20:00:00 +0100"));
  EXPECT_TRUE(result_DateTime.isValid());
}

TEST(UtilitiesTest, ParseISO8601DateTime) {
  QDateTime result_DateTime = Utilities::ParseISO8601DateTime(QString("2015-01-28T00:02:24+01:00"));
  EXPECT_TRUE(result_DateTime.isValid());
}
