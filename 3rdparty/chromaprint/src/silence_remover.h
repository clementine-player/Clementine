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

#ifndef CHROMAPRINT_SILENCE_REMOVER_H_
#define CHROMAPRINT_SILENCE_REMOVER_H_

#include "utils.h"
#include "audio_consumer.h"

namespace Chromaprint
{

	class SilenceRemover : public AudioConsumer
	{
	public:
		SilenceRemover(AudioConsumer *consumer);

		AudioConsumer *consumer() const
		{
			return m_consumer;
		}

		void set_consumer(AudioConsumer *consumer)
		{
			m_consumer = consumer;
		}

		bool Reset(int sample_rate, int num_channels);
		void Consume(short *input, int length);
		void Flush();

	private:
		CHROMAPRINT_DISABLE_COPY(SilenceRemover);

		bool m_start;
		AudioConsumer *m_consumer;
	};

};

#endif

