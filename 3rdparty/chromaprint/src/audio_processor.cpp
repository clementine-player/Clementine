/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2010  Lukas Lalinsky <lalinsky@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <assert.h>
#include <algorithm>
#include <stdio.h>
extern "C" {
#include "avresample/avcodec.h"
}
#include "debug.h"
#include "audio_processor.h"

using namespace std;
using namespace Chromaprint;

static const int kMinSampleRate = 1000;
static const int kMaxBufferSize = 1024 * 16;

// Resampler configuration
static const int kResampleFilterLength = 16;
static const int kResamplePhaseCount = 10;
static const int kResampleLinear = 0;
static const double kResampleCutoff = 0.8;

AudioProcessor::AudioProcessor(int sample_rate, AudioConsumer *consumer)
	: m_buffer_size(kMaxBufferSize),
	  m_target_sample_rate(sample_rate),
	  m_consumer(consumer),
	  m_resample_ctx(0)
{
	m_buffer = new short[kMaxBufferSize];
	m_buffer_offset = 0;
	m_resample_buffer = new short[kMaxBufferSize];
}

AudioProcessor::~AudioProcessor()
{
	if (m_resample_ctx) {
		av_resample_close(m_resample_ctx);
	}
	delete[] m_resample_buffer;
	delete[] m_buffer;
}

void AudioProcessor::LoadMono(short *input, int length)
{
	short *output = m_buffer + m_buffer_offset;
	while (length--) {
		*output++ = input[0];
		input++;
	}
}

void AudioProcessor::LoadStereo(short *input, int length)
{
	short *output = m_buffer + m_buffer_offset;
	while (length--) {
		*output++ = (input[0] + input[1]) / 2;
		input += 2;
	}
}

void AudioProcessor::LoadMultiChannel(short *input, int length)
{
	short *output = m_buffer + m_buffer_offset;
	while (length--) {
		long sum = 0;
		for (int i = 0; i < m_num_channels; i++) {
			sum += *input++;
		}
		*output++ = (short)(sum / m_num_channels);
	}
}

int AudioProcessor::Load(short *input, int length)
{
	assert(length >= 0);
	assert(m_buffer_offset <= m_buffer_size);
	length = min(length, m_buffer_size - m_buffer_offset);
	switch (m_num_channels) {
	case 1:
		LoadMono(input, length);
		break;
	case 2:
		LoadStereo(input, length);
		break;
	default:
		LoadMultiChannel(input, length);
		break;
	}
	m_buffer_offset += length;
	return length;
}

void AudioProcessor::Resample()
{
	if (!m_resample_ctx) {
		m_consumer->Consume(m_buffer, m_buffer_offset);
		m_buffer_offset = 0;
		return;
	}
	int consumed = 0;
	int length = av_resample(m_resample_ctx, m_resample_buffer, m_buffer, &consumed, m_buffer_offset, kMaxBufferSize, 1);
	if (length > kMaxBufferSize) {
		DEBUG() << "Chromaprint::AudioProcessor::Resample() -- Resampling overwrote output buffer.\n";
		length = kMaxBufferSize;
	}
	m_consumer->Consume(m_resample_buffer, length);
	int remaining = m_buffer_offset - consumed;
	if (remaining > 0) {
		copy(m_buffer + consumed, m_buffer + m_buffer_offset, m_buffer);
	}
	else if (remaining < 0) {
		DEBUG() << "Chromaprint::AudioProcessor::Resample() -- Resampling overread input buffer.\n";
		remaining = 0;
	}
	m_buffer_offset = remaining;
}


bool AudioProcessor::Reset(int sample_rate, int num_channels)
{
	if (num_channels <= 0) {
		DEBUG() << "Chromaprint::AudioProcessor::Reset() -- No audio channels.\n";
		return false;
	}
	if (sample_rate <= kMinSampleRate) {
		DEBUG() << "Chromaprint::AudioProcessor::Reset() -- Sample rate less "
				<< "than " << kMinSampleRate << " (" << sample_rate << ").\n";
		return false;
	}
	m_buffer_offset = 0;
	if (m_resample_ctx) {
		av_resample_close(m_resample_ctx);
		m_resample_ctx = 0;
	}
	if (sample_rate != m_target_sample_rate) {
		m_resample_ctx = av_resample_init(
			m_target_sample_rate, sample_rate,
			kResampleFilterLength,
			kResamplePhaseCount,
			kResampleLinear,
			kResampleCutoff);
	}
	m_num_channels = num_channels;
	return true;
}

void AudioProcessor::Consume(short *input, int length)
{
	assert(length >= 0);
	assert(length % m_num_channels == 0);
	length /= m_num_channels;
	while (length > 0) {
		int consumed = Load(input, length); 
		input += consumed * m_num_channels;		
		length -= consumed;
		if (m_buffer_size == m_buffer_offset) {
			Resample();
			if (m_buffer_size == m_buffer_offset) {
				DEBUG() << "Chromaprint::AudioProcessor::Consume() -- Resampling failed?\n";
				return;
			}
		}
	}
}

void AudioProcessor::Flush()
{
	if (m_buffer_offset) {
		Resample();
	}
}

