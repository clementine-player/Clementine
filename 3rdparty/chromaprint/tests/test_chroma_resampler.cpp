#include <gtest/gtest.h>
#include "image.h"
#include "image_builder.h"
#include "chroma_resampler.h"

using namespace std;
using namespace Chromaprint;

TEST(ChromaResampler, Test1) {
	Image image(12);
	ImageBuilder builder(&image);
	ChromaResampler resampler(2, &builder);
	double d1[] = { 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d2[] = { 1.0, 6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d3[] = { 2.0, 7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	std::vector<double> v1(d1, d1 + 12);
	std::vector<double> v2(d2, d2 + 12);
	std::vector<double> v3(d3, d3 + 12);
	resampler.Consume(v1);
	resampler.Consume(v2);
	resampler.Consume(v3);
	ASSERT_EQ(1, image.NumRows());
	EXPECT_EQ(0.5, image[0][0]);
	EXPECT_EQ(5.5, image[0][1]);
}

TEST(ChromaResampler, Test2) {
	Image image(12);
	ImageBuilder builder(&image);
	ChromaResampler resampler(2, &builder);
	double d1[] = { 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d2[] = { 1.0, 6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d3[] = { 2.0, 7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d4[] = { 3.0, 8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	std::vector<double> v1(d1, d1 + 12);
	std::vector<double> v2(d2, d2 + 12);
	std::vector<double> v3(d3, d3 + 12);
	std::vector<double> v4(d4, d4 + 12);
	resampler.Consume(v1);
	resampler.Consume(v2);
	resampler.Consume(v3);
	resampler.Consume(v4);
	ASSERT_EQ(2, image.NumRows());
	EXPECT_EQ(0.5, image[0][0]);
	EXPECT_EQ(5.5, image[0][1]);
	EXPECT_EQ(2.5, image[1][0]);
	EXPECT_EQ(7.5, image[1][1]);
}
