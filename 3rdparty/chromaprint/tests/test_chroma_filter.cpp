#include <gtest/gtest.h>
#include "image.h"
#include "image_builder.h"
#include "chroma_filter.h"

using namespace std;
using namespace Chromaprint;

TEST(ChromaFilter, Blur2) {
	double coefficients[] = { 0.5, 0.5 };
	Image image(12);
	ImageBuilder builder(&image);
	ChromaFilter filter(coefficients, 2, &builder);
	double d1[] = { 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d2[] = { 1.0, 6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d3[] = { 2.0, 7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	std::vector<double> v1(d1, d1 + 12);
	std::vector<double> v2(d2, d2 + 12);
	std::vector<double> v3(d3, d3 + 12);
	filter.Consume(v1);
	filter.Consume(v2);
	filter.Consume(v3);
	ASSERT_EQ(2, image.NumRows());
	EXPECT_EQ(0.5, image[0][0]);
	EXPECT_EQ(1.5, image[1][0]);
	EXPECT_EQ(5.5, image[0][1]);
	EXPECT_EQ(6.5, image[1][1]);
}

TEST(ChromaFilter, Blur3) {
	double coefficients[] = { 0.5, 0.7, 0.5 };
	Image image(12);
	ImageBuilder builder(&image);
	ChromaFilter filter(coefficients, 3, &builder);
	double d1[] = { 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d2[] = { 1.0, 6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d3[] = { 2.0, 7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d4[] = { 3.0, 8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	std::vector<double> v1(d1, d1 + 12);
	std::vector<double> v2(d2, d2 + 12);
	std::vector<double> v3(d3, d3 + 12);
	std::vector<double> v4(d4, d4 + 12);
	filter.Consume(v1);
	filter.Consume(v2);
	filter.Consume(v3);
	filter.Consume(v4);
	ASSERT_EQ(2, image.NumRows());
	EXPECT_FLOAT_EQ(1.7, image[0][0]);
	EXPECT_FLOAT_EQ(3.399999999999999, image[1][0]);
	EXPECT_FLOAT_EQ(10.199999999999999, image[0][1]);
	EXPECT_FLOAT_EQ(11.899999999999999, image[1][1]);
}

TEST(ChromaFilter, Diff) {
	double coefficients[] = { 1.0, -1.0 };
	Image image(12);
	ImageBuilder builder(&image);
	ChromaFilter filter(coefficients, 2, &builder);
	double d1[] = { 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d2[] = { 1.0, 6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double d3[] = { 2.0, 7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	std::vector<double> v1(d1, d1 + 12);
	std::vector<double> v2(d2, d2 + 12);
	std::vector<double> v3(d3, d3 + 12);
	filter.Consume(v1);
	filter.Consume(v2);
	filter.Consume(v3);
	ASSERT_EQ(2, image.NumRows());
	EXPECT_EQ(-1.0, image[0][0]);
	EXPECT_EQ(-1.0, image[1][0]);
	EXPECT_EQ(-1.0, image[0][1]);
	EXPECT_EQ(-1.0, image[1][1]);
}

