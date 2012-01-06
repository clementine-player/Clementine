#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "image.h"
#include "classifier.h"
#include "bit_string_writer.h"
#include "utils.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(BitStringWriter, OneByte)
{
	BitStringWriter writer;
	writer.Write(0, 2);
	writer.Write(1, 2);
	writer.Write(2, 2);
	writer.Write(3, 2);
	writer.Flush();

	char expected[] = { -28 };
	CheckString(writer.value(), expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(BitStringWriter, TwoBytesIncomplete)
{
	BitStringWriter writer;
	writer.Write(0, 2);
	writer.Write(1, 2);
	writer.Write(2, 2);
	writer.Write(3, 2);
	writer.Write(1, 2);
	writer.Flush();

	char expected[] = { -28, 1 };
	CheckString(writer.value(), expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(BitStringWriter, TwoBytesSplit)
{
	BitStringWriter writer;
	writer.Write(0, 3);
	writer.Write(1, 3);
	writer.Write(2, 3);
	writer.Write(3, 3);
	writer.Flush();

	char expected[] = { -120, 6 };
	CheckString(writer.value(), expected, sizeof(expected)/sizeof(expected[0]));
}
