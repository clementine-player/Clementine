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
#include <assert.h>
#include <math.h>
#include "chroma_filter.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

ChromaFilter::ChromaFilter(const double *coefficients, int length, FeatureVectorConsumer *consumer)
	: m_coefficients(coefficients),
	  m_length(length),
	  m_buffer(8),
	  m_result(12),
	  m_buffer_offset(0),
	  m_buffer_size(1),
	  m_consumer(consumer)
{
}

ChromaFilter::~ChromaFilter()
{
}

void ChromaFilter::Reset()
{
	m_buffer_size = 1;
	m_buffer_offset = 0;
}

void ChromaFilter::Consume(std::vector<double> &features)
{
	m_buffer[m_buffer_offset] = features;
	m_buffer_offset = (m_buffer_offset + 1) % 8;
	if (m_buffer_size >= m_length) {
		int offset = (m_buffer_offset + 8 - m_length) % 8;
		fill(m_result.begin(), m_result.end(), 0.0);
		for (int i = 0; i < 12; i++) {
			for (int j = 0; j < m_length; j++) {
				m_result[i] += m_buffer[(offset + j) % 8][i] * m_coefficients[j];
			}
		}
		m_consumer->Consume(m_result);
	}
	else {
		m_buffer_size++;
	}
}

