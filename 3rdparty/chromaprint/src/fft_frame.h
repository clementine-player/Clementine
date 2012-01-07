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

#ifndef CHROMAPRINT_FFT_FRAME_H_
#define CHROMAPRINT_FFT_FRAME_H_

#include <math.h>

namespace Chromaprint
{

	class FFTFrame
	{
	public:
		FFTFrame(int size) : m_size(size)
		{
			m_data = new double[size];
		}

		~FFTFrame()
		{
			delete[] m_data;
		}

		double Magnitude(int i) const
		{
			return sqrt(Energy(i));					
		}

		double Energy(int i) const
		{
			return m_data[i];
		}

		int size() const
		{
			return m_size;
		}

		double *data()
		{
			return m_data;
		}

	private:
		double *m_data;
		int m_size;
	};

};

#endif
