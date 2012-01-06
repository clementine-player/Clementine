#include <gtest/gtest.h>
#include "filter_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(FilterUtils, CompareSubtract) {
	double res = Subtract(2.0, 1.0);
	EXPECT_FLOAT_EQ(1.0, res);
}

TEST(FilterUtils, CompareSubtractLog) {
	double res = SubtractLog(2.0, 1.0);
	EXPECT_FLOAT_EQ(0.4054651, res);
}

TEST(FilterUtils, Filter0) {
	double data[] = {
		1.0, 2.0, 3.0,
		4.0, 5.0, 6.0,
		7.0, 8.0, 9.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);
	double res;
	res = Filter0(&integral_image, 0, 0, 1, 1, Subtract);
	EXPECT_FLOAT_EQ(1.0, res);
	res = Filter0(&integral_image, 0, 0, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(12.0, res);
	res = Filter0(&integral_image, 0, 0, 3, 3, Subtract);
	EXPECT_FLOAT_EQ(45.0, res);
	res = Filter0(&integral_image, 1, 1, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(28.0, res);
	res = Filter0(&integral_image, 2, 2, 1, 1, Subtract);
	EXPECT_FLOAT_EQ(9.0, res);
	res = Filter0(&integral_image, 0, 0, 3, 1, Subtract);
	EXPECT_FLOAT_EQ(12.0, res);
	res = Filter0(&integral_image, 0, 0, 1, 3, Subtract);
	EXPECT_FLOAT_EQ(6.0, res);
}

TEST(FilterUtils, Filter1) {
	double data[] = {
		1.0, 2.0, 3.0,
		3.0, 4.0, 5.0,
		6.0, 7.0, 8.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);
	double res;
	res = Filter1(&integral_image, 0, 0, 1, 1, Subtract);
	EXPECT_FLOAT_EQ(1.0, res); // 2 - 1
	res = Filter1(&integral_image, 0, 0, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(2.0, res); // 2+4 - 1+3
	res = Filter1(&integral_image, 0, 0, 3, 2, Subtract);
	EXPECT_FLOAT_EQ(3.0, res); // 2+4+7 - 1+3+6
}

TEST(FilterUtils, Filter2) {
	double data[] = {
		1.0, 2.0, 3.0,
		3.0, 4.0, 5.0,
		6.0, 7.0, 8.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);
	double res;
	res = Filter2(&integral_image, 0, 0, 2, 1, Subtract);
	EXPECT_FLOAT_EQ(2.0, res); // 3 - 1
	res = Filter2(&integral_image, 0, 0, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(4.0, res); // 3+4 - 1+2
	res = Filter2(&integral_image, 0, 0, 2, 3, Subtract);
	EXPECT_FLOAT_EQ(6.0, res); // 3+4+5 - 1+2+3
}

TEST(FilterUtils, Filter3) {
	double data[] = {
		1.0, 2.1, 3.4,
		3.1, 4.1, 5.1,
		6.0, 7.1, 8.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);
	double res;
	res = Filter3(&integral_image, 0, 0, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(0.1, res); // 2.1+3.1 - 1+4.1
	res = Filter3(&integral_image, 1, 1, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(0.1, res); // 4+8 - 5+7
	res = Filter3(&integral_image, 0, 1, 2, 2, Subtract);
	EXPECT_FLOAT_EQ(0.3, res); // 2.1+5.1 - 3.4+4.1
}

TEST(FilterUtils, Filter4) {
	double data[] = {
		1.0, 2.0, 3.0,
		3.0, 4.0, 5.0,
		6.0, 7.0, 8.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);
	double res;
	res = Filter4(&integral_image, 0, 0, 3, 3, Subtract);
	EXPECT_FLOAT_EQ(-13.0, res); // 2+4+7 - (1+3+6) - (3+5+8)
}

TEST(FilterUtils, Filter5) {
	double data[] = {
		1.0, 2.0, 3.0,
		3.0, 4.0, 5.0,
		6.0, 7.0, 8.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);
	double res;
	res = Filter5(&integral_image, 0, 0, 3, 3, Subtract);
	EXPECT_FLOAT_EQ(-15.0, res); // 3+4+5 - (1+2+3) - (6+7+8)
}

