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
#include "fft_lib_fftw3.h"

using namespace std;
using namespace Chromaprint;

FFTLib::FFTLib(int frame_size, double *window)
	: m_window(window),
	  m_frame_size(frame_size)
{
	m_input = (double *)fftw_malloc(sizeof(double) * frame_size);
	m_output = (double *)fftw_malloc(sizeof(double) * frame_size);
	m_plan = fftw_plan_r2r_1d(frame_size, m_input, m_output, FFTW_R2HC, FFTW_ESTIMATE);
}

FFTLib::~FFTLib()
{
	fftw_destroy_plan(m_plan);
	fftw_free(m_input);
	fftw_free(m_output);
}

void FFTLib::ComputeFrame(CombinedBuffer<short>::Iterator input, double *output)
{
	ApplyWindow(input, m_window, m_input, m_frame_size, 1.0);
	fftw_execute(m_plan);
	double *in_ptr = m_output;
	double *rev_in_ptr = m_output + m_frame_size - 1;
	output[0] = in_ptr[0] * in_ptr[0];
	output[m_frame_size / 2] = in_ptr[m_frame_size / 2] * in_ptr[m_frame_size / 2];
	in_ptr += 1;
	output += 1;
	for (int i = 1; i < m_frame_size / 2; i++) {
		*output++ = in_ptr[0] * in_ptr[0] + rev_in_ptr[0] * rev_in_ptr[0];
		in_ptr++;
		rev_in_ptr--;
	}
}
