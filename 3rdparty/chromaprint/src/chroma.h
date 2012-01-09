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

#ifndef CHROMAPRINT_CHROMA_H_
#define CHROMAPRINT_CHROMA_H_

#include <math.h>
#include <vector>
#include "utils.h"
#include "fft_frame_consumer.h"
#include "feature_vector_consumer.h"

namespace Chromaprint
{

	class Chroma : public FFTFrameConsumer
	{
	public:
		Chroma(int min_freq, int max_freq, int frame_size, int sample_rate, FeatureVectorConsumer *consumer);
		~Chroma();

		bool interpolate() const
		{
			return m_interpolate;
		}

		void set_interpolate(bool interpolate)
		{
			m_interpolate = interpolate;
		}

		void Reset();
		void Consume(const FFTFrame &frame);

	private:
		CHROMAPRINT_DISABLE_COPY(Chroma);

		void PrepareNotes(int min_freq, int max_freq, int frame_size, int sample_rate);

		bool m_interpolate;
		std::vector<char> m_notes;
		std::vector<double> m_notes_frac;
		int m_min_index;
		int m_max_index;
		std::vector<double> m_features;
		FeatureVectorConsumer *m_consumer;
	};

};

#endif
