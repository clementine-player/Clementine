#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "image.h"
#include "classifier.h"
#include "bit_string_reader.h"
#include "utils.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(BitStringReader, OneByte)
{
	char data[] = { -28 };
	BitStringReader reader(string(data, 1));

	ASSERT_EQ(0, reader.Read(2));
	ASSERT_EQ(1, reader.Read(2));
	ASSERT_EQ(2, reader.Read(2));
	ASSERT_EQ(3, reader.Read(2));
}

TEST(BitStringReader, TwoBytesIncomplete)
{
	char data[] = { -28, 1 };
	BitStringReader reader(string(data, 2));

	ASSERT_EQ(0, reader.Read(2));
	ASSERT_EQ(1, reader.Read(2));
	ASSERT_EQ(2, reader.Read(2));
	ASSERT_EQ(3, reader.Read(2));
	ASSERT_EQ(1, reader.Read(2));
}

TEST(BitStringReader, TwoBytesSplit)
{
	char data[] = { -120, 6 };
	BitStringReader reader(string(data, 2));

	ASSERT_EQ(0, reader.Read(3));
	ASSERT_EQ(1, reader.Read(3));
	ASSERT_EQ(2, reader.Read(3));
	ASSERT_EQ(3, reader.Read(3));
}
