#include <gtest/gtest.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <fstream>
#include "test_utils.h"
#include "silence_remover.h"
#include "audio_buffer.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

TEST(SilenceRemover, PassThrough)
{
	short samples[] = { 1, 2, 3, 4, 5, 6 };
	vector<short> data(samples, samples + 6);

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<SilenceRemover> processor(new SilenceRemover(buffer.get()));
	processor->Reset(44100, 1);
	processor->Consume(&data[0], data.size());
	processor->Flush();

	ASSERT_EQ(data.size(), buffer->data().size());
	for (size_t i = 0; i < data.size(); i++) {
		ASSERT_EQ(data[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}

TEST(SilenceRemover, RemoveLeadingSilence)
{
	short samples1[] = { 0, 0, 1, 2, 0, 4, 5, 0 };
	vector<short> data1(samples1, samples1 + 8);

	short samples2[] = { 1, 2, 0, 4, 5, 0 };
	vector<short> data2(samples2, samples2 + 6);

	boost::scoped_ptr<AudioBuffer> buffer(new AudioBuffer());
	boost::scoped_ptr<SilenceRemover> processor(new SilenceRemover(buffer.get()));
	processor->Reset(44100, 1);
	processor->Consume(&data1[0], data1.size());
	processor->Flush();

	ASSERT_EQ(data2.size(), buffer->data().size());
	for (size_t i = 0; i < data2.size(); i++) {
		ASSERT_EQ(data2[i], buffer->data()[i]) << "Signals differ at index " << i;
	}
}
