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

#ifndef CHROMAPRINT_FINGERPRINTER_H_
#define CHROMAPRINT_FINGERPRINTER_H_

#include <stdint.h>
#include <vector>
#include "image.h"
#include "audio_consumer.h"

namespace Chromaprint
{
	class ImageBuilder;
	class IntegralImage;
	class FFT;
	class Chroma;
	class ChromaNormalizer;
	class ChromaFilter;
	class AudioProcessor;
	class FingerprintCalculator;
	class FingerprinterConfiguration;

	class Fingerprinter : public AudioConsumer
	{
	public:
		Fingerprinter(FingerprinterConfiguration *config = 0);
		~Fingerprinter();

		/**
		 * Initialize the fingerprinting process.
		 */
		bool Start(int sample_rate, int num_channels);

		/**
		 * Process a block of raw audio data. Call this method as many times
		 * as you need. 
		 */
		void Consume(short *input, int length);

		/**
		 * Calculate the fingerprint based on the provided audio data.
		 */
		std::vector<int32_t> Finish();

	private:
		Image m_image;
		ImageBuilder *m_image_builder;
		Chroma *m_chroma;
		ChromaNormalizer *m_chroma_normalizer;
		ChromaFilter *m_chroma_filter;
		FFT *m_fft;
		AudioProcessor *m_audio_processor;
		FingerprintCalculator *m_fingerprint_calculator;
		FingerprinterConfiguration *m_config;
	};

};

#endif

