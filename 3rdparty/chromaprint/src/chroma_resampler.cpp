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
#include "chroma_resampler.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

ChromaResampler::ChromaResampler(int factor, FeatureVectorConsumer *consumer)
	: m_result(12, 0.0),
	  m_iteration(0),
	  m_factor(factor),
	  m_consumer(consumer)
{
}

ChromaResampler::~ChromaResampler()
{
}

void ChromaResampler::Reset()
{
	m_iteration = 0;
	fill(m_result.begin(), m_result.end(), 0.0);
}

void ChromaResampler::Consume(std::vector<double> &features)
{
	for (int i = 0; i < 12; i++) {
		m_result[i] += features[i];
	}
	m_iteration += 1;
	if (m_iteration == m_factor) {
		for (int i = 0; i < 12; i++) {
			m_result[i] /= m_factor;
		}
		m_consumer->Consume(m_result);
		Reset();
	}
}

