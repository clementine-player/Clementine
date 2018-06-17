#include "gtest/gtest.h"

#include "networkremote/zeroconf.h"

namespace {

static const char k64CharAscii[] =
    "aaaaaaaaaaaaaaaa"
    "aaaaaaaaaaaaaaaa"
    "aaaaaaaaaaaaaaaa"
    "aaaaaaaaaaaaaaaa";

static const char kShortMultiByteString[] =
    "我会说一点汉语";

static const char kLongMultiByteString[] =
    "我会说一点汉语"
    "我会说一点汉语"
    "我会说一点汉语"
    "我会说一点汉语";

TEST(ZeroconfTest, TruncatesAscii) {
  QByteArray truncated = Zeroconf::TruncateName(
      QString::fromLatin1(k64CharAscii));
  EXPECT_EQ(63, truncated.size());
  EXPECT_TRUE(truncated.endsWith('\0'));
}

TEST(ZeroconfTest, DoesNotTruncateShortMultiByteUTF8) {
  EXPECT_EQ(
      sizeof(kShortMultiByteString),
      Zeroconf::TruncateName(QString::fromUtf8(kShortMultiByteString)).size());
}

TEST(ZeroconfTest, TruncatesLongMultiByteUTF8) {
  QByteArray truncated = Zeroconf::TruncateName(
      QString::fromLatin1(kLongMultiByteString));
  EXPECT_LE(63, truncated.size());
  EXPECT_TRUE(truncated.endsWith('\0'));
}

}  // namespace
