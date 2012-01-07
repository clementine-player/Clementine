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

#ifndef CHROMAPRINT_CHROMA_RESAMPLER_H_
#define CHROMAPRINT_CHROMA_RESAMPLER_H_

#include <vector>
#include "image.h"
#include "feature_vector_consumer.h"

namespace Chromaprint
{
	
	class ChromaResampler : public FeatureVectorConsumer
	{
	public:
		ChromaResampler(int factor, FeatureVectorConsumer *consumer);
		~ChromaResampler();

		void Reset();
		void Consume(std::vector<double> &features);

		FeatureVectorConsumer *consumer() { return m_consumer; }
		void set_consumer(FeatureVectorConsumer *consumer) { m_consumer = consumer; }

	private:
		std::vector<double> m_result;
		int m_iteration;
		int m_factor;
		FeatureVectorConsumer *m_consumer;
	};

};

#endif
