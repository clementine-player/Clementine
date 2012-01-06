#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "test_utils.h"
#include "audio_processor.h"
#include "audio_buffer.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

TEST(AudioProcessor, Accessors)
{
	vector<short> data = LoadAudioFile("data/test_mono_44100.raw");

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<AudioBuffer> buffer2(new AudioBuffer());
	boost::scoped_ptr<AudioProcessor> processor(new AudioProcessor(44100, buffer.get()));

	EXPECT_EQ(44100, processor->target_sample_rate());
	EXPECT_EQ(buffer.get(), processor->consumer());

	processor->set_target_sample_rate(11025);
	EXPECT_EQ(11025, processor->target_sample_rate());

	processor->set_consumer(buffer2.get());
	EXPECT_EQ(buffer2.get(), processor->consumer());
}

TEST(AudioProcessor, PassThrough)
{
	vector<short> data = LoadAudioFile("data/test_mono_44100.raw");

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<AudioProcessor> processor(new AudioProcessor(44100, buffer.get()));
	processor->Reset(44100, 1);
	processor->Consume(&data[0], data.size());
	processor->Flush();

	ASSERT_EQ(data.size(), buffer->data().size());
	for (size_t i = 0; i < data.size(); i++) {
		ASSERT_EQ(data[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}

TEST(AudioProcessor, StereoToMono)
{
	vector<short> data1 = LoadAudioFile("data/test_stereo_44100.raw");
	vector<short> data2 = LoadAudioFile("data/test_mono_44100.raw");

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<AudioProcessor> processor(new AudioProcessor(44100, buffer.get()));
	processor->Reset(44100, 2);
	processor->Consume(&data1[0], data1.size());
	processor->Flush();

	ASSERT_EQ(data2.size(), buffer->data().size());
	for (size_t i = 0; i < data2.size(); i++) {
		ASSERT_EQ(data2[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}

TEST(AudioProcessor, ResampleMono)
{
	vector<short> data1 = LoadAudioFile("data/test_mono_44100.raw");
	vector<short> data2 = LoadAudioFile("data/test_mono_11025.raw");

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<AudioProcessor> processor(new AudioProcessor(11025, buffer.get()));
	processor->Reset(44100, 1);
	processor->Consume(&data1[0], data1.size());
	processor->Flush();

	ASSERT_EQ(data2.size(), buffer->data().size());
	for (size_t i = 0; i < data2.size(); i++) {
		ASSERT_EQ(data2[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}

TEST(AudioProcessor, ResampleMonoNonInteger)
{
	vector<short> data1 = LoadAudioFile("data/test_mono_44100.raw");
	vector<short> data2 = LoadAudioFile("data/test_mono_8000.raw");

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<AudioProcessor> processor(new AudioProcessor(8000, buffer.get()));
	processor->Reset(44100, 1);
	processor->Consume(&data1[0], data1.size());
	processor->Flush();

	ASSERT_EQ(data2.size(), buffer->data().size());
	for (size_t i = 0; i < data2.size(); i++) {
		ASSERT_EQ(data2[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}

TEST(AudioProcessor, StereoToMonoAndResample)
{
	vector<short> data1 = LoadAudioFile("data/test_stereo_44100.raw");
	vector<short> data2 = LoadAudioFile("data/test_mono_11025.raw");

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<AudioProcessor> processor(new AudioProcessor(11025, buffer.get()));
	processor->Reset(44100, 2);
	processor->Consume(&data1[0], data1.size());
	processor->Flush();

	ASSERT_EQ(data2.size(), buffer->data().size());
	for (size_t i = 0; i < data2.size(); i++) {
		ASSERT_EQ(data2[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}
