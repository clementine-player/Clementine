#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "image.h"
#include "filter.h"

using namespace std;
using namespace Chromaprint;

TEST(Filter, Filter0)
{
	Image image(2, 2);
	image[0][0] = 0.0;
	image[0][1] = 1.0;
	image[1][0] = 2.0;
	image[1][1] = 3.0;

	Filter flt1(0, 0, 1, 1);	
	IntegralImage integral_image(&image);
	ASSERT_FLOAT_EQ(0.0, flt1.Apply(&integral_image, 0));
	ASSERT_FLOAT_EQ(1.0986123, flt1.Apply(&integral_image, 1));
}

