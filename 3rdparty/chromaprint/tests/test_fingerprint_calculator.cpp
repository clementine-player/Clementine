#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "image.h"
#include "classifier.h"
#include "fingerprint_calculator.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

TEST(FingerprintCalculator, CalculateSubfingerprint)
{
	Image image(2, 2);
	image[0][0] = 0.0;
	image[0][1] = 1.0;
	image[1][0] = 2.0;
	image[1][1] = 3.0;

	Classifier classifiers[] = {
		Classifier(Filter(0, 0, 1, 1), Quantizer(0.01, 1.01, 1.5)),	
	};
	FingerprintCalculator calculator(classifiers, 1);

	IntegralImage integral_image(&image);
	EXPECT_EQ(GrayCode(0), calculator.CalculateSubfingerprint(&integral_image, 0));
	EXPECT_EQ(GrayCode(2), calculator.CalculateSubfingerprint(&integral_image, 1));
}

TEST(FingerprintCalculator, Calculate)
{
	Image image(2, 3);
	image[0][0] = 0.0;
	image[0][1] = 1.0;
	image[1][0] = 2.0;
	image[1][1] = 3.0;
	image[2][0] = 4.0;
	image[2][1] = 5.0;

	Classifier classifiers[] = {
		Classifier(Filter(0, 0, 1, 1), Quantizer(0.01, 1.01, 1.5)),	
	};
	FingerprintCalculator calculator(classifiers, 1);

	vector<int32_t> fp = calculator.Calculate(&image);
	ASSERT_EQ(3, fp.size());
	EXPECT_EQ(GrayCode(0), fp[0]);
	EXPECT_EQ(GrayCode(2), fp[1]);
	EXPECT_EQ(GrayCode(3), fp[2]);
}

