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

#include "fmpsparser.h"

#include <QtDebug>

class FMPSParserTest : public testing::Test {
protected:
  FMPSParser parser_;
};

TEST_F(FMPSParserTest, ParseFloats) {
  QVariant value;

  EXPECT_EQ(1, parser_.ParseValue("0", &value));
  EXPECT_EQ(QVariant::Double, value.type());
  EXPECT_EQ(0, value.toDouble());

  EXPECT_EQ(3, parser_.ParseValue("123", &value));
  EXPECT_EQ(QVariant::Double, value.type());
  EXPECT_EQ(123, value.toDouble());

  EXPECT_EQ(3, parser_.ParseValue("0.0", &value));
  EXPECT_EQ(QVariant::Double, value.type());
  EXPECT_EQ(0, value.toDouble());

  EXPECT_EQ(2, parser_.ParseValue("-1", &value));
  EXPECT_EQ(QVariant::Double, value.type());
  EXPECT_EQ(-1, value.toDouble());

  EXPECT_EQ(5, parser_.ParseValue("-1.23", &value));
  EXPECT_EQ(QVariant::Double, value.type());
  EXPECT_EQ(-1.23, value.toDouble());

  EXPECT_EQ(4, parser_.ParseValue("+123", &value));
  EXPECT_EQ(QVariant::Double, value.type());
  EXPECT_EQ(123, value.toDouble());

  parser_.ParseValue("1.", &value);
  EXPECT_NE(QVariant::Double, value.type());

  parser_.ParseValue("abc", &value);
  EXPECT_NE(QVariant::Double, value.type());
}

TEST_F(FMPSParserTest, ParseStrings) {
  QVariant value;

  EXPECT_EQ(3, parser_.ParseValue("abc", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("abc", value.toString());

  EXPECT_EQ(8, parser_.ParseValue("foo\\\\bar", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("foo\\bar", value.toString());

  EXPECT_EQ(8, parser_.ParseValue("foo\\:bar", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("foo:bar", value.toString());

  EXPECT_EQ(8, parser_.ParseValue("foo\\;bar", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("foo;bar", value.toString());

  EXPECT_EQ(12, parser_.ParseValue("foo\\\\\\:\\;bar", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("foo\\:;bar", value.toString());

  EXPECT_EQ(2, parser_.ParseValue("1.", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("1.", value.toString());

  EXPECT_EQ(5, parser_.ParseValue("1.abc", &value));
  EXPECT_EQ(QVariant::String, value.type());
  EXPECT_EQ("1.abc", value.toString());

  EXPECT_EQ(-1, parser_.ParseValue("foo\\bar", &value));

  EXPECT_EQ(-1, parser_.ParseValue("foo:bar", &value));

  EXPECT_EQ(-1, parser_.ParseValue("foo;bar", &value));
}

TEST_F(FMPSParserTest, ParseLists) {
  QVariantList value;

  EXPECT_EQ(3, parser_.ParseList("abc", &value));
  EXPECT_EQ(1, value.length());
  EXPECT_EQ("abc", value[0]);

  EXPECT_EQ(3, parser_.ParseList("123", &value));
  EXPECT_EQ(1, value.length());
  EXPECT_EQ(123, value[0]);

  EXPECT_EQ(8, parser_.ParseList("abc::def", &value));
  EXPECT_EQ(2, value.length());
  EXPECT_EQ("abc", value[0]);
  EXPECT_EQ("def", value[1]);

  EXPECT_EQ(13, parser_.ParseList("abc::def::ghi", &value));
  EXPECT_EQ(3, value.length());
  EXPECT_EQ("abc", value[0]);
  EXPECT_EQ("def", value[1]);
  EXPECT_EQ("ghi", value[2]);

  EXPECT_EQ(12, parser_.ParseList("ab\\:c::\\\\def", &value));
  EXPECT_EQ(2, value.length());
  EXPECT_EQ("ab:c", value[0]);
  EXPECT_EQ("\\def", value[1]);

  EXPECT_EQ(5, parser_.ParseList("abc::def:", &value));
  EXPECT_EQ(1, value.length());
  EXPECT_EQ("abc", value[0]);
}

TEST_F(FMPSParserTest, ParseListLists) {
  FMPSParser::Result value;

  EXPECT_EQ(8, parser_.ParseListList("abc::def", &value));
  EXPECT_EQ(1, value.length());
  EXPECT_EQ(2, value[0].length());
  EXPECT_EQ("abc", value[0][0]);
  EXPECT_EQ("def", value[0][1]);

  EXPECT_EQ(18, parser_.ParseListList("abc::def;;123::456", &value));
  EXPECT_EQ(2, value.length());
  EXPECT_EQ(2, value[0].length());
  EXPECT_EQ(2, value[1].length());
  EXPECT_EQ("abc", value[0][0]);
  EXPECT_EQ("def", value[0][1]);
  EXPECT_EQ(123, value[1][0]);
  EXPECT_EQ(456, value[1][1]);
}

TEST_F(FMPSParserTest, Parse) {
  EXPECT_TRUE(parser_.Parse("abc"));
  EXPECT_TRUE(parser_.Parse("abc::def"));
  EXPECT_TRUE(parser_.Parse("abc::def;;123::456;;foo::bar"));
  EXPECT_TRUE(parser_.Parse("1."));
  EXPECT_TRUE(parser_.Parse("1.abc"));

  EXPECT_FALSE(parser_.Parse("1:"));
  EXPECT_FALSE(parser_.Parse("1;"));
  EXPECT_FALSE(parser_.Parse("1:abc"));
  EXPECT_FALSE(parser_.Parse("abc;"));
}

TEST_F(FMPSParserTest, SpecExamples) {
  FMPSParser::Result expected;

  expected.clear();
  expected << (QVariantList() << "Alice Abba" << 0.6);
  expected << (QVariantList() << "Bob Beatles" << 0.8);

  ASSERT_TRUE(parser_.Parse("Alice Abba::0.6;;Bob Beatles::0.8"));
  EXPECT_EQ(expected, parser_.result());

  expected.clear();
  expected << (QVariantList() << "Rolling Stone" << "Ralph Gleason" << 0.83);
  expected << (QVariantList() << "musicOMH.com" << "FMPS_Nothing" << 0.76);
  expected << (QVariantList() << "Metacritic" << "FMPS_Nothing" << 0.8);
  expected << (QVariantList() << "FMPS_Nothing" << "Some Dude" << 0.9);

  ASSERT_TRUE(parser_.Parse("Rolling Stone::Ralph Gleason::0.83;;musicOMH.com::FMPS_Nothing::0.76;;Metacritic::FMPS_Nothing::0.8;;FMPS_Nothing::Some Dude::0.9"));
  EXPECT_EQ(expected, parser_.result());

  expected.clear();
  expected << (QVariantList() << "Amarok" << "AutoRate" << 0.52);
  expected << (QVariantList() << "VLC" << "Standard" << 0.6);
  expected << (QVariantList() << "QuodLibet" << "RatingPlugin:X" << 0.35);
  expected << (QVariantList() << "The Free Music Player Alliance" << "Rating Algorithm 1" << 0.5);

  ASSERT_TRUE(parser_.Parse("Amarok::AutoRate::0.52;;VLC::Standard::0.6;;QuodLibet::RatingPlugin\\:X::0.35;;The Free Music Player Alliance::Rating Algorithm 1::0.5"));
  EXPECT_EQ(expected, parser_.result());

  expected.clear();
  expected << (QVariantList() << "Willy Nelson" << "Guitar");
  expected << (QVariantList() << "Eric Clapton" << "Guitar (Backup)");
  expected << (QVariantList() << "B.B. King" << "Vocals");
  ASSERT_TRUE(parser_.Parse("Willy Nelson::Guitar;;Eric Clapton::Guitar (Backup);;B.B. King::Vocals"));
  EXPECT_EQ(expected, parser_.result());

  expected.clear();
  expected << (QVariantList() << "Alice Aardvark" << "[lyrics]");
  expected << (QVariantList() << "Bob Baboon" << "[lyrics]");
  expected << (QVariantList() << "http://www.lyricssite.net" << "[lyrics]");
  ASSERT_TRUE(parser_.Parse("Alice Aardvark::[lyrics];;Bob Baboon::[lyrics];;http\\://www.lyricssite.net::[lyrics]"));
  EXPECT_EQ(expected, parser_.result());

  expected.clear();
  expected << (QVariantList() << "Amarok" << "Album" << "2982ab29ef");
  expected << (QVariantList() << "AmarokUser" << "Compilation" << "My Compilation");
  expected << (QVariantList() << "Banshee" << "Compilation" << "ad8slpbzl229zier");
  expected << (QVariantList() << "FMPSAlliance" << "Album" << "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3");
  ASSERT_TRUE(parser_.Parse("Amarok::Album::2982ab29ef;;AmarokUser::Compilation::My Compilation;;Banshee::Compilation::ad8slpbzl229zier;;FMPSAlliance::Album::de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3"));
  EXPECT_EQ(expected, parser_.result());
}
