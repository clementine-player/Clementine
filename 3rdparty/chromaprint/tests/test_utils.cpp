#include <gtest/gtest.h>
#include <algorithm>
#include <limits>
#include "utils.h"

using namespace std;
using namespace Chromaprint;

TEST(Utils, PrepareHammingWindow) {
	double window_ex[10] = { 0.08, 0.187619556165, 0.460121838273, 0.77, 0.972258605562, 0.972258605562, 0.77, 0.460121838273, 0.187619556165, 0.08};
	double window[10];
	PrepareHammingWindow(window, window + 10);
	for (int i = 0; i < 10; i++) {
		EXPECT_FLOAT_EQ(window_ex[i], window[i]);
	}
}

TEST(Utils, ApplyWindow1) {
	double window_ex[10] = { 0.08, 0.187619556165, 0.460121838273, 0.77, 0.972258605562, 0.972258605562, 0.77, 0.460121838273, 0.187619556165, 0.08};
	double window[10];
	short input[10];
	double output[10];
	PrepareHammingWindow(window, window + 10);
	fill(input, input + 10, numeric_limits<short>::max());
	double scale = 1.0 / numeric_limits<short>::max();
	ApplyWindow(input, window, output, 10, scale);
	for (int i = 0; i < 10; i++) {
		EXPECT_FLOAT_EQ(window_ex[i], output[i]);
	}
}

TEST(Utils, ApplyWindow2) {
	double window[10];
	short input[10];
	double output[10];
	PrepareHammingWindow(window, window + 10);
	fill(input, input + 10, 0);
	double scale = 1.0 / numeric_limits<short>::max();
	ApplyWindow(input, window, output, 10, scale);
	for (int i = 0; i < 10; i++) {
		EXPECT_FLOAT_EQ(0.0, output[i]);
	}
}

TEST(Utils, Sum) {
	double data[] = { 0.1, 0.2, 0.4, 1.0 };
	EXPECT_FLOAT_EQ(1.7, Sum(data, data + 4));
}

TEST(Utils, EuclideanNorm) {
	double data[] = { 0.1, 0.2, 0.4, 1.0 };
	EXPECT_FLOAT_EQ(1.1, EuclideanNorm(data, data + 4));
}

TEST(Utils, NormalizeVector) {
	double data[] = { 0.1, 0.2, 0.4, 1.0 };
	double normalized_data[] = { 0.090909, 0.181818, 0.363636, 0.909091 };
	NormalizeVector(data, data + 4, EuclideanNorm<double *>, 0.01);
	for (int i = 0; i < 4; i++) {
		EXPECT_NEAR(normalized_data[i], data[i], 1e-5) << "Wrong data at index " << i;
	}
}

TEST(Utils, NormalizeVectorNearZero) {
	double data[] = { 0.0, 0.001, 0.002, 0.003 };
	NormalizeVector(data, data + 4, EuclideanNorm<double *>, 0.01);
	for (int i = 0; i < 4; i++) {
		EXPECT_FLOAT_EQ(0.0, data[i]) << "Wrong data at index " << i;
	}
}

TEST(Utils, NormalizeVectorZero) {
	double data[] = { 0.0, 0.0, 0.0, 0.0 };
	NormalizeVector(data, data + 4, EuclideanNorm<double *>, 0.01);
	for (int i = 0; i < 4; i++) {
		EXPECT_FLOAT_EQ(0.0, data[i]) << "Wrong data at index " << i;
	}
}

TEST(Utils, UnsignedToSigned) {
    EXPECT_EQ(numeric_limits<int32_t>::max(), UnsignedToSigned(0x7FFFFFFFU));
    EXPECT_EQ(-1, UnsignedToSigned(0xFFFFFFFFU));
    EXPECT_EQ(-2, UnsignedToSigned(0xFFFFFFFEU));
    EXPECT_EQ(numeric_limits<int32_t>::min(), UnsignedToSigned(0x80000000U));
    EXPECT_EQ(numeric_limits<int32_t>::min() + 1, UnsignedToSigned(0x80000001U));
}

TEST(Utils, IsNaN) {
    EXPECT_FALSE(IsNaN(0.0));
    EXPECT_TRUE(IsNaN(sqrt(-1.0)));
}
