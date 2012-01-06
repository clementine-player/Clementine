#include <gtest/gtest.h>
#include "quantizer.h"

using namespace std;
using namespace Chromaprint;

TEST(Quantizer, Quantize) {
	Quantizer q(0.0, 0.1, 0.3);
	EXPECT_EQ(0, q.Quantize(-0.1));
	EXPECT_EQ(1, q.Quantize(0.0));
	EXPECT_EQ(1, q.Quantize(0.03));
	EXPECT_EQ(2, q.Quantize(0.1));
	EXPECT_EQ(2, q.Quantize(0.13));
	EXPECT_EQ(3, q.Quantize(0.3));
	EXPECT_EQ(3, q.Quantize(0.33));
	EXPECT_EQ(3, q.Quantize(1000.0));
}
