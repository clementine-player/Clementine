/*
 * SpectralCentroidprint -- Audio fingerprinting toolkit
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

#ifndef CHROMAPRINT_SPECTRAL_CENTROID_H_
#define CHROMAPRINT_SPECTRAL_CENTROID_H_

#include <math.h>
#include <vector>
#include "utils.h"
#include "fft_frame_consumer.h"
#include "feature_vector_consumer.h"

namespace Chromaprint
{

	class SpectralCentroid : public FFTFrameConsumer
	{
	public:
		SpectralCentroid(int num_bands, int min_freq, int max_freq, int frame_size, int sample_rate, FeatureVectorConsumer *consumer);
		~SpectralCentroid();

		void Reset();
		void Consume(const FFTFrame &frame);

	protected:
		int NumBands() const { return m_bands.size() - 1; }
		int FirstIndex(int band) const { return m_bands[band]; }
		int LastIndex(int band) const { return m_bands[band + 1]; }

	private:
		CHROMAPRINT_DISABLE_COPY(SpectralCentroid);
		
		void PrepareBands(int num_bands, int min_freq, int max_freq, int frame_size, int sample_rate);

		std::vector<int> m_bands;
		std::vector<double> m_features;
		FeatureVectorConsumer *m_consumer;
	};

};

#endif
