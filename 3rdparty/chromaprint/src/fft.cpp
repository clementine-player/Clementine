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

#include <limits>
#include "utils.h"
#include "fft_lib.h"
#include "fft.h"
#include "debug.h"

using namespace std;
using namespace Chromaprint;

FFT::FFT(int frame_size, int overlap, FFTFrameConsumer *consumer)
	: m_window(new double[frame_size]),
	  m_buffer_offset(0),
	  m_buffer(new short[frame_size]),
	  m_frame(frame_size),
	  m_frame_size(frame_size),
	  m_increment(frame_size - overlap),
	  m_consumer(consumer)
{
	PrepareHammingWindow(m_window, m_window + frame_size);
	for (int i = 0; i < frame_size; i++) {
		m_window[i] /= numeric_limits<short>::max();
	}
	m_lib = new FFTLib(frame_size, m_window);
}

FFT::~FFT()
{
	delete m_lib;
	delete[] m_buffer;
	delete[] m_window;
}

void FFT::Reset()
{
	m_buffer_offset = 0;
}

void FFT::Consume(short *input, int length)
{
	// Special case, just pre-filling the buffer
	if (m_buffer_offset + length < m_frame_size) {
		copy(input, input + length, m_buffer + m_buffer_offset);
		m_buffer_offset += length;
		return;
	}
	// Apply FFT on the available data
	CombinedBuffer<short> combined_buffer(m_buffer, m_buffer_offset, input, length);
	while (combined_buffer.Size() >= m_frame_size) {
		m_lib->ComputeFrame(combined_buffer.Begin(), m_frame.data());
		m_consumer->Consume(m_frame);
		combined_buffer.Shift(m_increment);
	}
	// Copy the remaining input data to the internal buffer
	copy(combined_buffer.Begin(), combined_buffer.End(), m_buffer);
	m_buffer_offset = combined_buffer.Size();
}

