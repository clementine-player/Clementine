
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

#include "core/utilities.h"

#include <zlib.h>

#include <QDateTime>
#include <QtDebug>

#include "gtest/gtest.h"
#include "test_utils.h"

TEST(UtilitiesTest, HmacFunctions) {
  QString key("key");
  QString data("The quick brown fox jumps over the lazy dog");
  // Test Hmac Md5
  QByteArray result_hash_md5 =
      Utilities::HmacMd5(key.toLocal8Bit(), data.toLocal8Bit()).toHex();
  bool result_md5 =
      result_hash_md5 == QString("80070713463e7749b90c2dc24911e275");
  EXPECT_TRUE(result_md5);
  // Test Hmac Sha256
  QByteArray result_hash_sha256 =
      Utilities::HmacSha256(key.toLocal8Bit(), data.toLocal8Bit()).toHex();
  bool result_sha256 =
      result_hash_sha256 ==
      QString(
          "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
  EXPECT_TRUE(result_sha256);
}

TEST(UtilitiesTest, ParseRFC822DateTim) {
  QDateTime result_DateTime =
      Utilities::ParseRFC822DateTime(QString("22 Feb 2008 00:16:17 GMT"));
  EXPECT_TRUE(result_DateTime.isValid());
  result_DateTime = Utilities::ParseRFC822DateTime(
      QString("Thu, 13 Dec 2012 13:27:52 +0000"));
  EXPECT_TRUE(result_DateTime.isValid());
  result_DateTime = Utilities::ParseRFC822DateTime(
      QString("Mon, 12 March 2012 20:00:00 +0100"));
  EXPECT_TRUE(result_DateTime.isValid());
}

namespace {

// gzip, as Magnatune serves its database, with zlib's deflate.
QByteArray Gzip(const QByteArray& data) {
  z_stream stream = {};
  // 16 more than the largest window: gzip's header and trailer.
  EXPECT_EQ(Z_OK, deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                               MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY));
  QByteArray out(deflateBound(&stream, data.size()), '\0');
  stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
  stream.avail_in = data.size();
  stream.next_out = reinterpret_cast<Bytef*>(out.data());
  stream.avail_out = out.size();
  EXPECT_EQ(Z_STREAM_END, deflate(&stream, Z_FINISH));
  out.resize(stream.total_out);
  deflateEnd(&stream);
  return out;
}

}  // namespace

TEST(UtilitiesTest, Gunzip) {
  const QByteArray xml("<AllSongs><Track/></AllSongs>");
  QByteArray out;
  EXPECT_TRUE(Utilities::Gunzip(Gzip(xml), &out));
  EXPECT_EQ(xml.toStdString(), out.toStdString());

  // More than one buffer's worth.
  const QByteArray big = QByteArray("clementine").repeated(100000);
  EXPECT_TRUE(Utilities::Gunzip(Gzip(big), &out));
  // Not EXPECT_EQ, which would print a megabyte on failure.
  EXPECT_TRUE(big == out);
}

TEST(UtilitiesTest, GunzipRejectsWhatIsntWholeGzip) {
  QByteArray out("left over");
  EXPECT_FALSE(Utilities::Gunzip(QByteArray(), &out));
  EXPECT_TRUE(out.isEmpty());

  EXPECT_FALSE(Utilities::Gunzip("<AllSongs/>", &out));

  const QByteArray gzip = Gzip(QByteArray("clementine").repeated(1000));
  EXPECT_FALSE(Utilities::Gunzip(gzip.left(gzip.size() / 2), &out));
  EXPECT_TRUE(out.isEmpty());

  // zlib's own format isn't gzip.
  EXPECT_FALSE(Utilities::Gunzip(qCompress("clementine").mid(4), &out));
}
