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

#ifndef CHROMAPRINT_AUDIO_PROCESSOR_H_
#define CHROMAPRINT_AUDIO_PROCESSOR_H_

#include "utils.h"
#include "audio_consumer.h"

struct AVResampleContext;

namespace Chromaprint
{

	class AudioProcessor : public AudioConsumer
	{
	public:
		AudioProcessor(int sample_rate, AudioConsumer *consumer);
		virtual ~AudioProcessor();

		int target_sample_rate() const
		{
			return m_target_sample_rate;
		}

		void set_target_sample_rate(int sample_rate)
		{
			m_target_sample_rate = sample_rate;
		}

		AudioConsumer *consumer() const
		{
			return m_consumer;
		}

		void set_consumer(AudioConsumer *consumer)
		{
			m_consumer = consumer;
		}

		//! Prepare for a new audio stream
		bool Reset(int sample_rate, int num_channels);
		//! Process a chunk of data from the audio stream
		void Consume(short *input, int length);
		//! Process any buffered input that was not processed before and clear buffers
		void Flush();

	private:
		CHROMAPRINT_DISABLE_COPY(AudioProcessor);

		int Load(short *input, int length);
		void LoadMono(short *input, int length);
		void LoadStereo(short *input, int length);
		void LoadMultiChannel(short *input, int length);
		void Resample();

		short *m_buffer;
		short *m_resample_buffer;
		int m_buffer_offset;
		int m_buffer_size;
		int m_target_sample_rate;
		int m_num_channels;
		AudioConsumer *m_consumer;
		struct AVResampleContext *m_resample_ctx;
	};

};

#endif

