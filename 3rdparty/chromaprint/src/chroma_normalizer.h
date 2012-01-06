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

#ifndef CHROMAPRINT_CHROMA_NORMALIZER_H_
#define CHROMAPRINT_CHROMA_NORMALIZER_H_

#include <vector>
#include <algorithm>
#include "feature_vector_consumer.h"
#include "utils.h"

namespace Chromaprint
{

	class ChromaNormalizer : public FeatureVectorConsumer
	{
	public:
		ChromaNormalizer(FeatureVectorConsumer *consumer) : m_consumer(consumer) {}
		~ChromaNormalizer() {}
		void Reset() {}

		void Consume(std::vector<double> &features)
		{
			NormalizeVector(features.begin(), features.end(),
						    Chromaprint::EuclideanNorm<std::vector<double>::iterator>,
							0.01);
			m_consumer->Consume(features);
		}

	private:
		CHROMAPRINT_DISABLE_COPY(ChromaNormalizer);

		FeatureVectorConsumer *m_consumer;
	};

};

#endif
