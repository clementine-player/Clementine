#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "fingerprint_decompressor.h"
#include "utils.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(FingerprintDecompressor, OneItemOneBit)
{
	int32_t expected[] = { 1 };
	char data[] = { 0, 0, 0, 1, 1 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}


TEST(FingerprintDecompressor, OneItemThreeBits)
{
	int32_t expected[] = { 7 };
	char data[] = { 0, 0, 0, 1, 73, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExcept)
{
	int32_t expected[] = { 1<<6 };
	char data[] = { 0, 0, 0, 1, 7, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, OneItemOneBitExcept2)
{
	int32_t expected[] = { 1<<8 };
	char data[] = { 0, 0, 0, 1, 7, 2 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, TwoItems)
{
	int32_t expected[] = { 1, 0 };
	char data[] = { 0, 0, 0, 2, 65, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

TEST(FingerprintDecompressor, TwoItemsNoChange)
{
	int32_t expected[] = { 1, 1 };
	char data[] = { 0, 0, 0, 2, 1, 0 };

	int algorithm = 1;
	vector<int32_t> value = DecompressFingerprint(string(data, NELEMS(data)), &algorithm);
	CheckFingerprints(value, expected, NELEMS(expected));
	ASSERT_EQ(0, algorithm);
}

