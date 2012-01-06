#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "audio_processor.h"
#include "test_utils.h"
#include "audio_processor.h"
#include "chroma.h"
#include "chroma_normalizer.h"
#include "chroma_resampler.h"
#include "fft.h"
#include "audio_processor.h"
#include "image.h"
#include "image_builder.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

static const int SAMPLE_RATE = 11025;
static const int FRAME_SIZE = 4096;
static const int OVERLAP = FRAME_SIZE - FRAME_SIZE / 3;// 2720;
static const int MIN_FREQ = 28;
static const int MAX_FREQ = 3520;
static const int MAX_FILTER_WIDTH = 20;

TEST(Chromaprint, BasicImage)
{
	vector<short> data = LoadAudioFile("data/test_stereo_44100.raw");

	Chromaprint::Image image(12);
	Chromaprint::ImageBuilder image_builder(&image);
	Chromaprint::ChromaNormalizer chroma_normalizer(&image_builder);
	Chromaprint::Chroma chroma(MIN_FREQ, MAX_FREQ, FRAME_SIZE, SAMPLE_RATE, &chroma_normalizer);
	Chromaprint::FFT fft(FRAME_SIZE, OVERLAP, &chroma);
	Chromaprint::AudioProcessor processor(SAMPLE_RATE, &fft);

	processor.Reset(44100, 2);
	processor.Consume(&data[0], data.size());
	processor.Flush();

	double chromagram[][12] = {
		{ 0.155444, 0.268618, 0.474445, 0.159887, 0.1761, 0.423511, 0.178933, 0.34433, 0.360958, 0.30421, 0.200217, 0.17072 },
		{ 0.159809, 0.238675, 0.286526, 0.166119, 0.225144, 0.449236, 0.162444, 0.371875, 0.259626, 0.483961, 0.24491, 0.17034 },
		{ 0.156518, 0.271503, 0.256073, 0.152689, 0.174664, 0.52585, 0.141517, 0.253695, 0.293199, 0.332114, 0.442906, 0.170459 },
		{ 0.154183, 0.38592, 0.497451, 0.203884, 0.362608, 0.355691, 0.125349, 0.146766, 0.315143, 0.318133, 0.172547, 0.112769 },
		{ 0.201289, 0.42033, 0.509467, 0.259247, 0.322772, 0.325837, 0.140072, 0.177756, 0.320356, 0.228176, 0.148994, 0.132588 },
		{ 0.187921, 0.302804, 0.46976, 0.302809, 0.183035, 0.228691, 0.206216, 0.35174, 0.308208, 0.233234, 0.316017, 0.243563 },
		{ 0.213539, 0.240346, 0.308664, 0.250704, 0.204879, 0.365022, 0.241966, 0.312579, 0.361886, 0.277293, 0.338944, 0.290351 },
		{ 0.227784, 0.252841, 0.295752, 0.265796, 0.227973, 0.451155, 0.219418, 0.272508, 0.376082, 0.312717, 0.285395, 0.165745 },
		{ 0.168662, 0.180795, 0.264397, 0.225101, 0.562332, 0.33243, 0.236684, 0.199847, 0.409727, 0.247569, 0.21153, 0.147286 },
		{ 0.0491864, 0.0503369, 0.130942, 0.0505802, 0.0694409, 0.0303877, 0.0389852, 0.674067, 0.712933, 0.05762, 0.0245158, 0.0389336 },
		{ 0.0814379, 0.0312366, 0.240546, 0.134609, 0.063374, 0.0466124, 0.0752175, 0.657041, 0.680085, 0.0720311, 0.0249404, 0.0673359 },
		{ 0.139331, 0.0173442, 0.49035, 0.287237, 0.0453947, 0.0873279, 0.15423, 0.447475, 0.621502, 0.127166, 0.0355933, 0.141163 },
		{ 0.115417, 0.0132515, 0.356601, 0.245902, 0.0283943, 0.0588233, 0.117077, 0.499376, 0.715366, 0.100398, 0.0281382, 0.0943482 },
		{ 0.047297, 0.0065354, 0.181074, 0.121455, 0.0135504, 0.030693, 0.0613105, 0.631705, 0.73548, 0.0550565, 0.0128093, 0.0460393 },
	};

	ASSERT_EQ(14, image.NumRows()) << "Numbers of rows doesn't match";
	for (int y = 0; y < 14; y++) {
		for (int x = 0; x < 12; x++) {
			EXPECT_NEAR(chromagram[y][x], image[y][x], 1e-5)
				<< "Image not equal at (" << x << ", " << y << ")";
		}
	}
}
