#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "chromaprint.h"

using namespace std;

TEST(API, Test2SilenceFp)
{
	short zeroes[1024];
	fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	chromaprint_start(ctx, 44100, 1);
	for (int i = 0; i < 130; i++) {
		chromaprint_feed(ctx, zeroes, 1024);
	}

	char *fp;

	chromaprint_finish(ctx);
	chromaprint_get_fingerprint(ctx, &fp);

	ASSERT_EQ(18, strlen(fp));
	EXPECT_EQ(string("AQAAA0mUaEkSRZEGAA"), string(fp));
}

TEST(API, Test2SilenceRawFp)
{
	short zeroes[1024];
	fill(zeroes, zeroes + 1024, 0);

	ChromaprintContext *ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_TEST2);
	chromaprint_start(ctx, 44100, 1);
	for (int i = 0; i < 130; i++) {
		chromaprint_feed(ctx, zeroes, 1024);
	}

	int32_t *fp;
	int length;

	chromaprint_finish(ctx);
	chromaprint_get_raw_fingerprint(ctx, (void **)&fp, &length);

	ASSERT_EQ(3, length);
	EXPECT_EQ(627964279, fp[0]);
	EXPECT_EQ(627964279, fp[1]);
	EXPECT_EQ(627964279, fp[2]);
}

TEST(API, TestEncodeFingerprint)
{
	int32_t fingerprint[] = { 1, 0 };
	char expected[] = { 55, 0, 0, 2, 65, 0 };

	char *encoded;
	int encoded_size;
	chromaprint_encode_fingerprint(fingerprint, 2, 55, (void **)&encoded, &encoded_size, 0);

	ASSERT_EQ(6, encoded_size);
	for (int i = 0; i < encoded_size; i++) {
		ASSERT_EQ(expected[i], encoded[i]) << "Different at " << i;
	}

	free(encoded);
}

TEST(API, TestEncodeFingerprintBase64)
{
	int32_t fingerprint[] = { 1, 0 };
	char expected[] = "NwAAAkEA";

	char *encoded;
	int encoded_size;
	chromaprint_encode_fingerprint(fingerprint, 2, 55, (void **)&encoded, &encoded_size, 1);

	ASSERT_EQ(8, encoded_size);
	ASSERT_STREQ(expected, encoded);

	free(encoded);
}

TEST(API, TestDecodeFingerprint)
{
	char data[] = { 55, 0, 0, 2, 65, 0 };

	int32_t *fingerprint;
	int size;
	int algorithm;
	chromaprint_decode_fingerprint(data, 6, (void **)&fingerprint, &size, &algorithm, 0);

	ASSERT_EQ(2, size);
	ASSERT_EQ(55, algorithm);
	ASSERT_EQ(1, fingerprint[0]);
	ASSERT_EQ(0, fingerprint[1]);
}

