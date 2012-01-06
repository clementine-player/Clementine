#include <vector>
#include <algorithm>
#include <gtest/gtest.h>
#include "fft_frame.h"
#include "chroma.h"

using namespace std;
using namespace Chromaprint;

class FeatureVectorBuffer : public FeatureVectorConsumer
{
public:
	void Consume(std::vector<double> &features)
	{
		m_features = features;
	}

	std::vector<double> m_features;
};

TEST(Chroma, NormalA) {
	FeatureVectorBuffer buffer;
	Chroma chroma(10, 510, 256, 1000, &buffer);
	FFTFrame frame(128);
	std::fill(frame.data(), frame.data() + frame.size(), 0.0);
	frame.data()[113] = 1.0;
	chroma.Consume(frame);
	ASSERT_EQ(12, buffer.m_features.size());
	double expected_features[12] = {
		1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	};
	for (int i = 0; i < 12; i++) {
		EXPECT_NEAR(expected_features[i], buffer.m_features[i], 0.0001) << "Different value at index " << i;
	}
}

TEST(Chroma, NormalGSharp) {
	FeatureVectorBuffer buffer;
	Chroma chroma(10, 510, 256, 1000, &buffer);
	FFTFrame frame(128);
	std::fill(frame.data(), frame.data() + frame.size(), 0.0);
	frame.data()[112] = 1.0;
	chroma.Consume(frame);
	ASSERT_EQ(12, buffer.m_features.size());
	double expected_features[12] = {
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
	};
	for (int i = 0; i < 12; i++) {
		EXPECT_NEAR(expected_features[i], buffer.m_features[i], 0.0001) << "Different value at index " << i;
	}
}

TEST(Chroma, NormalB) {
	FeatureVectorBuffer buffer;
	Chroma chroma(10, 510, 256, 1000, &buffer);
	FFTFrame frame(128);
	std::fill(frame.data(), frame.data() + frame.size(), 0.0);
	frame.data()[64] = 1.0; // 250 Hz
	chroma.Consume(frame);
	ASSERT_EQ(12, buffer.m_features.size());
	double expected_features[12] = {
		0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	};
	for (int i = 0; i < 12; i++) {
		EXPECT_NEAR(expected_features[i], buffer.m_features[i], 0.0001) << "Different value at index " << i;
	}
}

TEST(Chroma, InterpolatedB) {
	FeatureVectorBuffer buffer;
	Chroma chroma(10, 510, 256, 1000, &buffer);
	chroma.set_interpolate(true);
	FFTFrame frame(128);
	std::fill(frame.data(), frame.data() + frame.size(), 0.0);
	frame.data()[64] = 1.0;
	chroma.Consume(frame);
	ASSERT_EQ(12, buffer.m_features.size());
	double expected_features[12] = {
		0.0, 0.286905, 0.713095, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	};
	for (int i = 0; i < 12; i++) {
		EXPECT_NEAR(expected_features[i], buffer.m_features[i], 0.0001) << "Different value at index " << i;
	}
}

TEST(Chroma, InterpolatedA) {
	FeatureVectorBuffer buffer;
	Chroma chroma(10, 510, 256, 1000, &buffer);
	chroma.set_interpolate(true);
	FFTFrame frame(128);
	std::fill(frame.data(), frame.data() + frame.size(), 0.0);
	frame.data()[113] = 1.0;
	chroma.Consume(frame);
	ASSERT_EQ(12, buffer.m_features.size());
	double expected_features[12] = {
		0.555242, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.444758,
	};
	for (int i = 0; i < 12; i++) {
		EXPECT_NEAR(expected_features[i], buffer.m_features[i], 0.0001) << "Different value at index " << i;
	}
}

TEST(Chroma, InterpolatedGSharp) {
	FeatureVectorBuffer buffer;
	Chroma chroma(10, 510, 256, 1000, &buffer);
	chroma.set_interpolate(true);
	FFTFrame frame(128);
	std::fill(frame.data(), frame.data() + frame.size(), 0.0);
	frame.data()[112] = 1.0;
	chroma.Consume(frame);
	ASSERT_EQ(12, buffer.m_features.size());
	double expected_features[12] = {
		0.401354, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.598646,
	};
	for (int i = 0; i < 12; i++) {
		EXPECT_NEAR(expected_features[i], buffer.m_features[i], 0.0001) << "Different value at index " << i;
	}
}

