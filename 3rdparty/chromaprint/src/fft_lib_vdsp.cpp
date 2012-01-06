/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2011  Lukas Lalinsky <lalinsky@gmail.com>
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
#include "fft_lib_vdsp.h"

using namespace std;
using namespace Chromaprint;

FFTLib::FFTLib(int frame_size, double *window)
	: m_window(window),
	  m_frame_size(frame_size)
{
	double log2n = log2(frame_size);
	assert(log2n == int(log2n));
	m_log2n = int(log2n);
	m_input = new float[frame_size];
	m_a.realp = new float[frame_size / 2];
	m_a.imagp = new float[frame_size / 2];
	m_setup = vDSP_create_fftsetup(m_log2n, 0);
}

FFTLib::~FFTLib()
{
	vDSP_destroy_fftsetup(m_setup);
	delete[] m_a.realp;
	delete[] m_a.imagp;
	delete[] m_input;
}

void FFTLib::ComputeFrame(CombinedBuffer<short>::Iterator input, double *output)
{
	ApplyWindow(input, m_window, m_input, m_frame_size, 1.0);
	// XXX we can avoid this ctoz call by changing ApplyWindow, is it worth it?
	vDSP_ctoz((DSPComplex *)m_input, 2, &m_a, 1, m_frame_size / 2); 
	vDSP_fft_zrip(m_setup, &m_a, 1, m_log2n, FFT_FORWARD);
	output[0] = m_a.realp[0] * m_a.realp[0];
	output[m_frame_size / 2] = m_a.imagp[0] * m_a.imagp[0];
	output += 1;
	for (int i = 1; i < m_frame_size / 2; i++) {
		*output++ = m_a.realp[i] * m_a.realp[i] + m_a.imagp[i] * m_a.imagp[i];
	}
}

