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

#ifndef CHROMAPRINT_FFT_H_
#define CHROMAPRINT_FFT_H_

#include <math.h>
#include "utils.h"
#include "fft_frame.h"
#include "fft_frame_consumer.h"
#include "audio_consumer.h"
#include "combined_buffer.h"

namespace Chromaprint
{
	class FFTLib;

	class FFT : public AudioConsumer
	{
	public:
		FFT(int frame_size, int overlap, FFTFrameConsumer *consumer);
		~FFT();

		int FrameSize() const { return m_frame_size; }
		int Overlap() const { return m_frame_size - m_increment; }

		void Reset();
		void Consume(short *input, int length);

	private:
		CHROMAPRINT_DISABLE_COPY(FFT);

		double *m_window;
		int m_buffer_offset;
		short *m_buffer;
		FFTFrame m_frame;
		int m_frame_size;
		int m_increment;
		FFTLib *m_lib;
		FFTFrameConsumer *m_consumer;
	};

};

#endif
