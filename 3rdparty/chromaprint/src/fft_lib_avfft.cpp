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

#include "utils.h"
#include "fft_lib_avfft.h"

using namespace std;
using namespace Chromaprint;

FFTLib::FFTLib(int frame_size, double *window)
	: m_window(window),
	  m_frame_size(frame_size)
{
	m_input = (float *)av_mallocz(sizeof(float) * frame_size);
	int bits = -1;
	while (frame_size) {
		bits++;
		frame_size >>= 1;
	}
	m_rdft_ctx = av_rdft_init(bits, DFT_R2C);
}

FFTLib::~FFTLib()
{
	av_rdft_end(m_rdft_ctx);
	av_free(m_input);
}

void FFTLib::ComputeFrame(CombinedBuffer<short>::Iterator input, double *output)
{
	ApplyWindow(input, m_window, m_input, m_frame_size, 1.0);
	av_rdft_calc(m_rdft_ctx, m_input);
	float *in_ptr = m_input;
	output[0] = in_ptr[0] * in_ptr[0];
	output[m_frame_size / 2] = in_ptr[1] * in_ptr[1];
	output += 1;
	in_ptr += 2;
	for (int i = 1; i < m_frame_size / 2; i++) {
		*output++ = in_ptr[0] * in_ptr[0] + in_ptr[1] * in_ptr[1];
		in_ptr += 2;
	}
}

