#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "image.h"
#include "classifier.h"
#include "fingerprint_compressor.h"
#include "utils.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(FingerprintCompressor, OneItemOneBit)
{
	FingerprintCompressor compressor;

	int32_t fingerprint[] = { 1 };
	string value = compressor.Compress(vector<int32_t>(fingerprint, fingerprint + 1));

	char expected[] = { 0, 0, 0, 1, 1 };
	CheckString(value, expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(FingerprintCompressor, OneItemThreeBits)
{
	FingerprintCompressor compressor;

	int32_t fingerprint[] = { 7 };
	string value = compressor.Compress(vector<int32_t>(fingerprint, fingerprint + 1));

	char expected[] = { 0, 0, 0, 1, 73, 0 };
	CheckString(value, expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(FingerprintCompressor, OneItemOneBitExcept)
{
	FingerprintCompressor compressor;

	int32_t fingerprint[] = { 1<<6 };
	string value = compressor.Compress(vector<int32_t>(fingerprint, fingerprint + 1));

	char expected[] = { 0, 0, 0, 1, 7, 0 };
	CheckString(value, expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(FingerprintCompressor, OneItemOneBitExcept2)
{
	FingerprintCompressor compressor;

	int32_t fingerprint[] = { 1<<8 };
	string value = compressor.Compress(vector<int32_t>(fingerprint, fingerprint + 1));

	char expected[] = { 0, 0, 0, 1, 7, 2 };
	CheckString(value, expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(FingerprintCompressor, TwoItems)
{
	FingerprintCompressor compressor;

	int32_t fingerprint[] = { 1, 0 };
	string value = compressor.Compress(vector<int32_t>(fingerprint, fingerprint + 2));

	char expected[] = { 0, 0, 0, 2, 65, 0 };
	CheckString(value, expected, sizeof(expected)/sizeof(expected[0]));
}

TEST(FingerprintCompressor, TwoItemsNoChange)
{
	FingerprintCompressor compressor;

	int32_t fingerprint[] = { 1, 1 };
	string value = compressor.Compress(vector<int32_t>(fingerprint, fingerprint + 2));

	char expected[] = { 0, 0, 0, 2, 1, 0 };
	CheckString(value, expected, sizeof(expected)/sizeof(expected[0]));
}
