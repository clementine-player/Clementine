/*
 * Spectrumprint -- Audio fingerprinting toolkit
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
#include <math.h>
#include "fft_frame.h"
#include "utils.h"
#include "spectrum.h"

using namespace std;
using namespace Chromaprint;

Spectrum::Spectrum(int num_bands, int min_freq, int max_freq, int frame_size, int sample_rate, FeatureVectorConsumer *consumer)
	: m_bands(num_bands + 1),
	  m_features(num_bands),
	  m_consumer(consumer)
{
	PrepareBands(num_bands, min_freq, max_freq, frame_size, sample_rate);
}

Spectrum::~Spectrum()
{
}

void Spectrum::PrepareBands(int num_bands, int min_freq, int max_freq, int frame_size, int sample_rate)
{
    double min_bark = FreqToBark(min_freq);
    double max_bark = FreqToBark(max_freq);
    double band_size = (max_bark - min_bark) / num_bands;

    int min_index = FreqToIndex(min_freq, frame_size, sample_rate);
    //int max_index = FreqToIndex(max_freq, frame_size, sample_rate);

    m_bands[0] = min_index;
    double prev_bark = min_bark;

    for (int i = min_index, b = 0; i < frame_size / 2; i++) {
        double freq = IndexToFreq(i, frame_size, sample_rate);
        double bark = FreqToBark(freq);
        if (bark - prev_bark > band_size) {
            b += 1;
            prev_bark = bark;
            m_bands[b] = i;
            if (b >= num_bands) {
                break;
            }
        }
    }
}

void Spectrum::Reset()
{
}

void Spectrum::Consume(const FFTFrame &frame)
{
	for (int i = 0; i < NumBands(); i++) {
		int first = FirstIndex(i);
		int last = LastIndex(i);
		double numerator = 0.0;
		double denominator = 0.0;
		for (int j = first; j < last; j++) {
			double s = frame.Energy(j);
			numerator += j * s;
			denominator += s;
		}
		m_features[i] = denominator / (last - first);
	}
	m_consumer->Consume(m_features);
}

